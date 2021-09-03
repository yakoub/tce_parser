#include "tce_parse.h"
#include "debug.h"
#include "data.h"
#include <signal.h>

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
      game.players[i].idx = Empty;
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

  debug_info(DBGLVL + 1, "init game: host %s map %s type %d\n", 
    game->hostname, game->mapname, game->gametype);
}

void player_disconnect(const char* line, GameScore *game) {
  int idx;

  sscanf(line, ": %d", &idx);
  for(int i = 0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == idx) {
      game->players[i].idx *= -1;
      debug_info(DBGLVL, "Player: idx %d gone\n", idx);
      break;
    }
  }
}

void player_connect(const char* line, GameScore *game) {
  int idx;

  sscanf(line, ": %d", &idx);
  int empty = -1, *slot_idx = NULL;
  for(int i = 0; i < MAX_PLAYERS; i++) {
    slot_idx = &game->players[i].idx;
    if (*slot_idx == idx) {
      return;
    }
    if (*slot_idx == idx * -1) {
      *slot_idx = idx;
      return;
    }
    if (game->players[i].idx == Empty && empty == -1) {
      empty = i;
    }
  }
  Player *pl = &game->players[empty];
  pl->idx = idx;
  strcpy(pl->name, "unknown");
  debug_info(DBGLVL, "Player: idx %d new\n", game->players[empty].idx);
}

void player_info(const char* line, GameScore *game) {
  static Player pl;

  sscanf(line, ": %d n\\%64[^\\]\\t\\%d\\c", &pl.idx, pl.name, &pl.team);

  int found = Empty, empty = Empty;
  for(int i = 0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == pl.idx) {
      found = i;
      break;
    }
    if (game->players[i].idx == Empty) {
      empty = i;
    }
  }
  if (found == Empty) {
    if (empty != Empty) {
      found = empty;
    }
    else {
      return;
    }
  }

  Player *p_ref = &game->players[found];
  p_ref->idx = pl.idx;
  p_ref->team = pl.team;
  strcpy(p_ref->name, pl.name);

  debug_info(DBGLVL, "Player: name %s idx %d team %d\n", 
    game->players[found].name, p_ref->idx, p_ref->team);
}

void team_score(const char* line, GameScore *game) {
  sscanf(line, ":%d blue%d", &game->team_red, &game->team_blue);
}

void player_score(const char* line, GameScore *game) {
  Player buff;
  sscanf(line, ": %d ping: %d client: %d ", 
    &buff.score, &buff.ping, &buff.idx);

  for (int i=0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == buff.idx
      || game->players[i].idx == buff.idx * -1) {
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
  Player *p = NULL;
  int idx, ignore, mask, hits, kills, deaths, headshots;
  static char buff[256];
  sscanf(line, ": %d %d %d %256[^#]",
    &idx, &ignore, &mask, buff);

  for (int i=0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == idx) {
      p = game->players + i;
    }
  }
  if (!p) {
    debug_info(DBGLVL, "player %d not found for  weapons\n", idx);
    return;
  }

  debug_info(DBGLVL, "mask %d, ", mask);
  int weapons = 0;
  while(mask > 0 && weapons < 16) {
    if (mask & 1) {
      weapons++;
    }
    mask = mask >> 1;
  }
  debug_info(DBGLVL, "weapons %d\n", weapons);
  
  p->kills = p->deaths = p->headshots = 0;
  int read;
  while (weapons--) {
    debug_info(DBGLVL, "wpns buff %s\n", buff);
    read = sscanf(buff, " %d %d %d %d %d%256[^#]", 
      &hits, &ignore, &kills, &deaths, &headshots, buff);
    if (read < 5) {
      debug_info(DBGLVL, "weapon stats error for %d, buff %s", idx, buff);
      break;
    }
    if (hits > 0) { // not ctf related
      p->kills += kills;
      p->deaths += deaths;
      p->headshots += headshots;
    }
  }
  sscanf(buff, " %d %d", &p->damage_given, &p->damage_recieved);

  debug_info(DBGLVL, "player %d kills %d, deaths %d given %d received %d\n", 
    p->idx, p->kills, p->deaths, p->damage_given, p->damage_recieved);
}

void shutdown_game(const char* line, GameScore *game) {
  if (game->player_scores > 0) {
    save_game(game);
  }
  else {
    for (int i=0; i < MAX_PLAYERS; i++) {
      if (game->players[i].idx < 0) {
        game->players[i].idx = Empty;
      }
    }
  }
  debug_info(DBGLVL + 1, "game shutdown, scores %d\n", game->player_scores);
}


void tce_parse_action(int sig) {
  tce_parse("00:00 ShutdownGame:");
  data_init(CLOSE);
  debug_info(DBGLVL + 1, "game shutdown by signal");
  exit(EXIT_SUCCESS);
}

void tce_parse_init() {
  struct sigaction sa;
  sa.sa_handler = tce_parse_action;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);

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

