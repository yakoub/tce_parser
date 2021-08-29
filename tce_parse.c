#include "tce_parse.h"
#include "debug.h"
#include "data.h"

#define DBGLVL 2

typedef struct {
  const char* op;
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
  static char ignore[16], op[64], buff[BUFF_SIZE];
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

  sscanf(line, "%16s %64[^:]%[^\n]", ignore, op, buff);

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
    sscanf(at, "mapname\\%64[^\\]", game->mapname);
  }
  else {
    strcpy(game->mapname, "not found\0");
  }

  at = strstr(line, "g_gametype");
  if (at) {
    sscanf(at, "g_gametype\\%d", &game->gametype);
  }
  else {
    game->gametype = -1;
  }

  at = strstr(line, "sv_hostname");
  if (at) {
    sscanf(at, "sv_hostname\\%64[^\\]", game->hostname);
  }
  else {
    strcpy(game->hostname, "not found\0");
  }

  game->player_scores = 0;

  debug_info(DBGLVL, "init game: host %s map %s type %d\n", 
    game->hostname, game->mapname, game->gametype);
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
  static Player pl;

  sscanf(line, ": %d n\\%64[^\\]\\t\\%d\\c", &pl.idx, pl.name, &pl.team);

  int found = -1, empty = -1;
  for(int i = 0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == pl.idx) {
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

  Player *p_ref = &game->players[found];
  p_ref->idx = pl.idx;
  p_ref->team = pl.team;
  strcpy(p_ref->name, pl.name);

  debug_info(DBGLVL, "Player: name %s idx %d team %d\n", 
    p_ref->name, p_ref->idx, p_ref->team);
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
      debug_info(DBGLVL, "player %d register score %d, total %d\n", 
        buff.idx, buff.score, game->player_scores);
      return;
    }
  }
  // report error
  debug_info(DBGLVL, "player %d score not assigned\n", buff.idx);
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

      debug_info(DBGLVL, "player %d kills %d, deaths %d\n", 
        buff.idx, buff.kills, buff.deaths);
      return;
    }
  }
}

void shutdown_game(const char* line, GameScore *game) {
  if (game->player_scores > 0) {
    save_game(game);

  }
  debug_info(DBGLVL, "game shutdown, scores %d\n", game->player_scores);
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

