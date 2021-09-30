#include "sync.h"
#include "debug.h"
#include "tce_parse.h"
#include "data.h"
#include "conf.h"
#include <signal.h>

#define DBGLVL 4

typedef struct {
  long game_pos;
  long console_pos;
} dir_watch;

typedef struct {
  int wd;
  FILE* game_log;
  FILE* console_log;
  FILE* pos_track;
  long game_pos;
  long console_pos;
  const char *path;
  GameScore *game;
} game_dir;

#define SLOTS 20

game_dir game_slots[SLOTS];

void sync_write_pos(game_dir *slot) {
  static dir_watch dwatch;
  
  if (!slot->pos_track) {
    debug_info(DBGLVL, "pos_track not open at writing\n");
    return;
  }

  dwatch.game_pos = slot->game_pos;
  dwatch.console_pos = slot->console_pos;

  rewind(slot->pos_track);
  fwrite(&dwatch, sizeof(dir_watch), 1, slot->pos_track);
  fflush(slot->pos_track);
  
  debug_info(DBGLVL+1, "wrote pos wd = %d game = %d\n", slot->wd, dwatch.game_pos);
}

void sync_read_pos(game_dir *slot) {
  static dir_watch dwatch;

  if (!slot->pos_track) {
    slot->game_pos = 0;
    slot->console_pos = 0;
    debug_info(DBGLVL+1, "pos_track not open at reading\n");
    return;
  }
  rewind(slot->pos_track);

  dwatch.game_pos = 0; dwatch.console_pos = 0;
  fread(&dwatch, sizeof(dir_watch), 1, slot->pos_track);

  slot->game_pos = dwatch.game_pos;
  slot->console_pos = dwatch.console_pos;

  debug_info(DBGLVL, "read pos wd = %d game = %d\n", slot->wd, slot->game_pos);
}

void sync_logs_action(int sig) {

  for (int i=0; i < SLOTS; i++) {
    if (game_slots[i].wd != -1) {
      tce_parse("00:00 ShutdownGame:", game_slots[i].game);
      sync_write_pos(game_slots + i);
    }
  }
  data_init(CLOSE);
  debug_info(DBGLVL+1, "game shutdown by signal");
  exit(EXIT_SUCCESS);
}

void sync_logs_init() {
  struct sigaction sa;
  sa.sa_handler = sync_logs_action;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);

  for (int i=0; i < SLOTS; i++) {
    game_slots[i].wd = -1;
  }
}

int sync_logs_find(int wd) {
  int empty = -1;
  for (int i=0; i < SLOTS; i++) {
    if (game_slots[i].wd == wd) {
      debug_info(DBGLVL+1, "wd %d found\n", wd);
      return i;
    }
    if (game_slots[i].wd == empty) {
      empty = i;
    }
  }
  debug_info(DBGLVL, "wd %d not found, assign to %d\n", wd, empty);
  return empty;
}

void sync_logs_assign(int wd, const PathConfig* config) {
  char filename[128];
  for (int i=0; i<SLOTS; i++) {
    if (game_slots[i].wd == -1) {
      game_slots[i].wd = wd;
      
      strncpy(filename, config->path, 100);
      game_slots[i].game_log = fopen(strcat(filename, "/game.log"), "r");
      strncpy(filename, config->path, 100);
      game_slots[i].console_log = fopen(strcat(filename, "/etconsole.log"), "r");
      strncpy(filename, config->path, 100);
      game_slots[i].pos_track = fopen(strcat(filename, "/.tce_watch"), "r+b");
      if (!game_slots[i].pos_track) {
        debug_info(DBGLVL, "wd %d pos_track created\n", wd);
        game_slots[i].pos_track = fopen(filename, "w+b");
      }
      sync_read_pos(game_slots + i);

      game_slots[i].path = config->path;
      game_slots[i].game = malloc(sizeof(GameScore));
      strncpy(game_slots[i].game->hostname, config->hostname, 64);
      tce_parse_game_init(game_slots[i].game);
      debug_info(DBGLVL+1, "wd %d assinged to %d\n", wd, i);
      return;
    }
  }
  debug_info(DBGLVL, "wd %d not assinged \n", wd);
}

void sync_logs_close() {
  for (int i=0; i<SLOTS; i++) {
    if (game_slots[i].wd != -1) {
      fclose(game_slots[i].game_log);
      fclose(game_slots[i].console_log);
      sync_write_pos(game_slots + i);
      fclose(game_slots[i].pos_track);
    }
  }
}

void sync_logs_rewind(const char* name, int wd) {
  int at = sync_logs_find(wd);
  game_dir *slot = &game_slots[at];
  char filename[128];
  strncpy(filename, slot->path, 100);

  if (strcmp(name, "game.log") == 0) {
    if (slot->game_log) {
      fclose(slot->game_log);
    }
    slot->game_log = fopen(strcat(filename, "/game.log"), "r");
    slot->game_pos = 0;
    debug_info(DBGLVL , "game.log rewind for wd %d", wd);
  }
  if (strcmp(name, "etconsole.log") == 0) {
    if (slot->console_log) {
      fclose(slot->console_log);
    }
    slot->console_log = fopen(strcat(filename, "/etconsole.log"), "r");
    slot->console_pos = 0;
    debug_info(DBGLVL , "etconsole.log rewind for wd %d", wd);
  }
  sync_write_pos(slot);
}

void sync_logs(const char *name, int wd) {
  int at = sync_logs_find(wd);
  static char buff[BUFF_SIZE];
  int seek_ret;
  long int *pos;
  FILE *logfile;
  void (*parse)(const char*, GameScore*);

  if (strcmp(name, "game.log") == 0) {
    logfile = game_slots[at].game_log;
    pos = &game_slots[at].game_pos;
    parse = tce_parse;
    debug_info(DBGLVL+1, "game.log for ws=%d\n", wd);
  }
  else {
    if (strcmp(name, "etconsole.log") == 0) {
      logfile = game_slots[at].console_log;
      pos = &game_slots[at].console_pos;
      parse = tce_parse_guid;
      debug_info(DBGLVL, "etconsole.log for ws=%d\n", wd);
    }
    else {
      return;
    }
  }

  if (!logfile) {
    return;
  }
  fseek(logfile, *pos, SEEK_SET);

  while (fgets(buff, BUFF_SIZE, logfile)) {
    parse(buff, game_slots[at].game);
  }
  debug_info(DBGLVL, "wd %d, start pos %d", wd, *pos);
  *pos = ftell(logfile);
  debug_info(DBGLVL, ", end pos %d\n", *pos);
  sync_write_pos(game_slots + at);
}
