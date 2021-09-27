#include<stdio.h>
#include<string.h>

typedef struct {
  long game_pos;
  long console_pos;
} dir_watch;

void main(int argc, char **argv) {
  FILE *pos_track;
  char filename[128];
  dir_watch dwatch;

  if (argc < 2) {
    printf("no given paths\n");
    return;
  }

  strncpy(filename, argv[1], 100);
  strcat(filename, "/.tce_watch");
  pos_track = fopen(filename, "r+b");
  if (!pos_track) {
    printf("failed to open file\n");
    return;
  }
  fread(&dwatch, sizeof(dir_watch), 1, pos_track);
  printf("game = %ld console = %ld\n", dwatch.game_pos, dwatch.console_pos);
  dwatch.console_pos = 0;
  rewind(pos_track);
  fwrite(&dwatch, sizeof(dir_watch), 1, pos_track);
  printf("reset console pos\n");
}
