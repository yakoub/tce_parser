#include "sync.h"
#include "debug.h"
#include "tce_parse.h"

#define DBGLVL 2

typedef struct {
  int wd;
  FILE* game_log;
  FILE* console_log;
  long game_pos;
  long console_pos;
  const char *path;
} game_dir;

#define SLOTS 20

game_dir game_slots[SLOTS];

void sync_logs_init() {
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
      game_slots[i].game_log = fopen(strcat(filename, "/game.log"), "r");
      strncpy(filename, path, 50);
      game_slots[i].console_log = fopen(strcat(filename, "/console.log"), "r");
      game_slots[i].game_pos = 0;
      game_slots[i].console_pos = 0;
      game_slots[i].path = path;
      debug_info(DBGLVL, "wd %d assinged to %d\n", wd, i);
      break;
    }
  }
}

void sync_logs_close() {
  for (int i=0; i<SLOTS; i++) {
    if (game_slots[i].wd != -1) {
      fclose(game_slots[i].game_log);
      fclose(game_slots[i].console_log);
    }
  }
}

void sync_logs_rewind(const char* name, int wd) {
  int at = sync_logs_find(wd);
  game_dir *slot = &game_slots[at];
  char filename[32];
  strcpy(filename, slot->path);

  if (strcmp(name, "game.log") == 0) {
    if (slot->game_log) {
      fclose(slot->game_log);
    }
    slot->game_log = fopen(strcat(filename, "/game.log"), "r");
    slot->game_pos = 0;
    debug_info(DBGLVL , "game.log rewind for wd %s", wd);
  }
  if (strcmp(name, "console.log") == 0) {
    if (slot->console_log) {
      fclose(slot->console_log);
    }
    slot->console_log = fopen(strcat(filename, "/console.log"), "r");
    slot->console_pos = 0;
    debug_info(DBGLVL , "console.log rewind for wd %s", wd);
  }
}

void sync_logs(const char *name, int wd) {
  int at = sync_logs_find(wd);
  static char buff[BUFF_SIZE];
  int seek_ret;
  long int *pos;
  FILE *logfile;

  if (strcmp(name, "game.log") == 0) {
    logfile = game_slots[at].game_log;
    pos = &game_slots[at].game_pos;
  }
  else {
    if (strcmp(name, "console.log") == 0) {
      logfile = game_slots[at].console_log;
      pos = &game_slots[at].console_pos;
      debug_info(DBGLVL, "console.log for ws=%d", wd);
    }
    else {
      return;
    }
  }

  if (!logfile) {
    return;
  }
  seek_ret = fseek(logfile, *pos, SEEK_SET);
  if (seek_ret != 0) {
    rewind(logfile);
  }
  while (fgets(buff, BUFF_SIZE, logfile)) {
    tce_parse(buff);
  }
  debug_info(DBGLVL, "wd %d, start pos %d", wd, *pos);
  *pos = ftell(logfile);
  debug_info(DBGLVL, ", end pos %d\n", *pos);
}
