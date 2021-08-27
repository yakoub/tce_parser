#include "tce_parse.h"
#include "data.h"

typedef struct {
  char* op;
  void (*parse)(const char*, GameScore*);
} route;

#define ROUTES 8

route router[ROUTES];

enum router_ids {
  ClientDisconnect,
  ClientConnect,
  InitGame,
  ShutdownGame,
  ClientUserinfoChanged,
  Team,
  WeaponStats,
  Score
};

void tce_parse_init();

void tce_parse(const char* line) {

  static GameScore game;
  static char players_init = 0;
  static char ignore[16], op[32], buff[BUFF_SIZE];
  static char router_init = 0;

  if (!router_init) {
    tce_parse_init();
    router_init = 1;
  }
  if (!players_init) {
    for(int i = 0; i < MAX_PLAYERS; i++) {
      game.players[i].idx = -1;
    }
    players_init = 1;
  }

  sscanf(line, "%16s %32[^:]%[^\n]", ignore, op, buff);

  for(int i = 0; i < ROUTES; i++) {
    if (strcmp(router[i].op, op) == 0) {
      router[i].parse(buff, &game);
      break;
    }
  }
}

void init_game(const char* line, GameScore *game) {
  char *at = strstr(line, "mapname");
  if (at) {
    sscanf(at, "mapname\\%32[^\\]", game->mapname);
  }
  else {
    strcpy(game->mapname, "not found\0");
  }

  at = strstr(at, "sv_hostname");
  if (at) {
    sscanf(at, "sv_hostname\\%32[^\\]", game->hostname);
  }
  else {
    strcpy(game->hostname, "not found\0");
  }

  game->player_scores = 0;
}

void player_disconnect(const char* line, GameScore *game) {
  int idx;

  sscanf(line, ": %d n", &idx);
  for(int i = 0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == idx) {
      game->players[i].idx = -1;
      break;
    }
  }
}

void player_connect(const char* line, GameScore *game) {
  int idx;

  sscanf(line, ": %d n", &idx);
  int empty = -1;
  for(int i = 0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == idx) {
      return;
    }
    if (game->players[i].idx == empty) {
      empty = i;
    }
  }
  Player *pl = &game->players[empty];
  pl->idx = idx;
  strcpy(pl->name, "unknown");
  pl->team = 4;
}

void player_info(const char* line, GameScore *game) {
  int team, idx;
  sscanf(line, ": %d n", &idx);

  int found = -1, empty = -1;
  for(int i = 0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == idx) {
      found = i;
      break;
    }
    if (game->players[i].idx == empty) {
      empty = i;
    }
  }
  if (found == -1) {
    found = empty;
  }

  Player *pl = &game->players[found];

  sscanf(line, ": %d n\\%32[^\\]\\t\\%d\\c", &idx, pl->name, &team);
  pl->team = team;
  pl->idx = idx;
}

void team_score(const char* line, GameScore *game) {
  sscanf(line, ":%d blue%d", &game->team_red, &game->team_blue);
}

void player_score(const char* line, GameScore *game) {
  Player buff;
  sscanf(line, ": %d ping: %d client: %d ", 
    &buff.score, &buff.ping, &buff.idx);

  for (int i=0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == buff.idx) {
      game->players[i].score = buff.score;
      game->players[i].ping = buff.ping;
      game->player_scores++;
      return;
    }
  }
  // report error
}

void weapons_stats(const char* line, GameScore *game) {
  Player buff;
  int i;
  sscanf(line, ": %d %d %d %d %d %d %d %d",
    &buff.idx, &i, &i, &i, &i, &buff.kills, &buff.deaths, &buff.headshots);

  for (int i=0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == buff.idx) {
      game->players[i].kills = buff.kills;
      game->players[i].deaths = buff.deaths;
      game->players[i].headshots = buff.headshots;
      return;
    }
  }
}

void shutdown_game(const char* line, GameScore *game) {
  if (game->player_scores > 0) {
    save_game(game);
  }
}

void tce_parse_init() {
  char *operations[ROUTES] = {
    [ClientDisconnect]="ClientDisconnect",
    [ClientConnect]="ClientConnect",
    [InitGame]="InitGame",
    [ShutdownGame]="ShutdownGame",
    [ClientUserinfoChanged]="ClientUserinfoChanged",
    [Team]="red",
    [WeaponStats]="WeaponStats",
    [Score]="score"
  };

  void (*parsers[ROUTES])(const char*, GameScore*) = {
    [ClientDisconnect]=player_disconnect,
    [ClientConnect]=player_connect,
    [InitGame]=init_game,
    [ShutdownGame]=shutdown_game,
    [ClientUserinfoChanged]=player_info,
    [Team]=team_score,
    [WeaponStats]=weapons_stats,
    [Score]=player_score,
  };

  for (int i=0; i< ROUTES; i++) {
    router[i].op = operations[i];
    router[i].parse = parsers[i];
  }
}

