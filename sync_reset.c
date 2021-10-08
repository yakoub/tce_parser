#include <stdio.h>
#include <string.h>
#include "conf.h"

typedef struct {
  long game_pos;
  long console_pos;
} dir_watch;

void main(int argc, char **argv) {
  FILE *pos_track, *game_log;
  char filename[128];
  dir_watch dwatch;

  int path_count;
  PathConfig *conf = config_paths(&path_count);

  char report = 0;
  if (argc == 2) {
    if (strcmp(argv[1], "report") == 0) {
      report = 1;
    }
    else {
      path_count = 1;
      conf[0].path = argv[1];
    }
  }
  
  for (int i=0; i<path_count; i++) {
    strncpy(filename, conf[i].path, 100);
    strcat(filename, "/.tce_watch");
    pos_track = fopen(filename, "r+b");
    if (!pos_track) {
      printf("failed to open file %s\n", filename);
      continue;
    }
    fread(&dwatch, sizeof(dir_watch), 1, pos_track);
    printf("game = %ld console = %ld\n", dwatch.game_pos, dwatch.console_pos);
    if (report) {
      strncpy(filename, conf[i].path, 100);
      strcat(filename, "/game.log");
      game_log = fopen(filename, "r");
      fseek(game_log, 0, SEEK_END);
      printf("report game = %ld\n", ftell(game_log));
      fclose(game_log);
    }
    else {
      dwatch.console_pos = 0;
      rewind(pos_track);
      fwrite(&dwatch, sizeof(dir_watch), 1, pos_track);
    }
    fclose(pos_track);
  }
}
