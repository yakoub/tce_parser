#include "sync.h"
#include "tce_parse.h"

FILE* game_log;
FILE* game_log_out;
FILE* console_log;
long game_pos;
long console_pos;

void sync_logs_init() {
  game_log = fopen("./watch.d/game.log", "r");
  console_log = fopen("./watch.d/console.log", "r");
  game_pos = 0;
  console_pos = 0;
}

void sync_logs_close() {
  fclose(game_log);
  fclose(console_log);
  printf("closed\n");
}

void sync_logs_rewind(const char* name) {
  if (strcmp(name, "game.log") == 0) {
    if (game_log) {
      fclose(game_log);
    }
    game_log = fopen("./watch.d/game.log", "r");
    game_pos = 0;
  }
  if (strcmp(name, "console.log") == 0) {
    if (console_log) {
      fclose(console_log);
    }
    console_log = fopen("./watch.d/console.log", "r");
    console_pos = 0;
  }
}

void sync_logs() {

  static char buff[BUFF_SIZE];
  int seek_ret;

  if (!game_log) {
    return;
  }
  seek_ret = fseek(game_log, game_pos, SEEK_SET);
  if (seek_ret != 0) {
    rewind(game_log);
  }
  while (fgets(buff, BUFF_SIZE, game_log)) {
    tce_parse(buff);
  }
  game_pos = ftell(game_log);

  fseek(console_log, console_pos, SEEK_SET);
  while (fgets(buff, BUFF_SIZE, console_log)) {
  //  puts(buff);
  }
  console_pos = ftell(console_log);

}
