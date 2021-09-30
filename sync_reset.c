#include <stdio.h>
#include <string.h>
#include "conf.h"

typedef struct {
  long game_pos;
  long console_pos;
} dir_watch;

void main(int argc, char **argv) {
  FILE *pos_track;
  char filename[128];
  dir_watch dwatch;

  int path_count;
  PathConfig *conf = config_paths(&path_count);

  if (argc == 2) {
    path_count = 1;
    conf[0].path = argv[1];
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
    dwatch.console_pos = 0;
    rewind(pos_track);
    fwrite(&dwatch, sizeof(dir_watch), 1, pos_track);
  }
}
