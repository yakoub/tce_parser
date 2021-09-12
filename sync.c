#include "sync.h"
#include "debug.h"
#include "tce_parse.h"
#include "data.h"
#include <signal.h>

#define DBGLVL 4

typedef struct {
  int wd;
  FILE* console_log;
  long console_pos;
  const char *path;
  GameScore *game;
} game_dir;

#define SLOTS 20

game_dir game_slots[SLOTS];


void sync_logs_action(int sig) {

  for (int i=0; i < SLOTS; i++) {
    if (game_slots[i].wd != -1) {
      tce_parse("00:00 ShutdownGame:", game_slots[i].game);
    }
  }
  data_init(CLOSE);
  debug_info(DBGLVL + 1, "game shutdown by signal");
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
      debug_info(DBGLVL, "wd %d found\n", wd);
      return i;
    }
    if (game_slots[i].wd == empty) {
      empty = i;
    }
  }
  debug_info(DBGLVL, "wd %d not found, assign to %d\n", wd, empty);
  return empty;
}

void sync_logs_assign(int wd, const char* path) {
  char filename[64];
  for (int i=0; i<SLOTS; i++) {
    if (game_slots[i].wd == -1) {
      game_slots[i].wd = wd;
      strncpy(filename, path, 50);
      game_slots[i].console_log = fopen(strcat(filename, "/etconsole.log"), "r");
      game_slots[i].console_pos = 0;
      game_slots[i].path = path;
      game_slots[i].game = malloc(sizeof(GameScore));
      tce_parse_game_init(game_slots[i].game);

      debug_info(DBGLVL, "wd %d assinged to %d\n", wd, i);
      return;
    }
  }
  debug_info(DBGLVL, "wd %d not assinged \n", wd);
}

void sync_logs_close() {
  for (int i=0; i<SLOTS; i++) {
    if (game_slots[i].wd != -1) {
      fclose(game_slots[i].console_log);
    }
  }
}

void sync_logs_rewind(const char* name, int wd) {
  int at = sync_logs_find(wd);
  game_dir *slot = &game_slots[at];
  char filename[32];

  if (strcmp(name, "etconsole.log") == 0) {
    if (slot->console_log) {
      fclose(slot->console_log);
    }
    strcpy(filename, slot->path);
    slot->console_log = fopen(strcat(filename, "/etconsole.log"), "r");
    slot->console_pos = 0;
    debug_info(DBGLVL , "etconsole.log rewind for wd %s", wd);
  }
}

void sync_logs(const char *name, int wd) {
  int at = sync_logs_find(wd);
  static char buff[BUFF_SIZE];
  int seek_ret;
  long int *pos;
  FILE *logfile = NULL;


  if (strcmp(name, "etconsole.log") == 0) {
    logfile = game_slots[at].console_log;
    pos = &game_slots[at].console_pos;
    debug_info(DBGLVL+1, "etconsole.log for ws=%d\n", wd);
  }
  else {
    return;
  }

  if (!logfile) {
    return;
  }
  seek_ret = fseek(logfile, *pos, SEEK_SET);
  if (seek_ret != 0) {
    rewind(logfile);
  }
  while (fgets(buff, BUFF_SIZE, logfile)) {
    tce_parse(buff, game_slots[at].game);
  }
  debug_info(DBGLVL, "wd %d, start pos %d", wd, *pos);
  *pos = ftell(logfile);
  debug_info(DBGLVL, ", end pos %d\n", *pos);
}
