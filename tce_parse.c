#include "tce_parse.h"
#include "debug.h"
#include <signal.h>

#define DBGLVL 4

typedef struct {
  const char* op;
  void (*parse)(const char*, GameScore*);
} route;

#define ROUTES 12

route router[ROUTES];

enum router_ids {
  InitGame,
  Map,
  ShutdownGame,
  ClientDisconnect,
  ClientConnect,
  ClientBegin,
  Userinfo,
  ClientUserinfoChanged,
  Team,
  LegacyTeam,
  WeaponStats,
  Score
};

void tce_parse_init();

void tce_parse_guid(const char* line, GameScore *game) {
  static char ignore[16], op[64];
  sscanf(line, "%16s %64[^:]", ignore, op);

  if (
    strcmp("Userinfo", op) == 0
    || strcmp("ClientConnect", op) == 0
    ) {
    debug_info(DBGLVL, "console called: %s\n", op);
    tce_parse(line, game);
  }
}

void tce_parse(const char* line, GameScore *game) {

  static char ignore[16], op[64], buff[BUFF_SIZE];
  static char router_init = 0;

  if (!router_init) {
    tce_parse_init();
    router_init = 1;
  }

  sscanf(line, "%16s %64[^:]%[^\n]", ignore, op, buff);

  for(int i = 0; i < ROUTES; i++) {
    if (strcmp(router[i].op, op) == 0) {
      router[i].parse(buff, game);
      break;
    }
  }
}

void init_game(const char* line, GameScore *game) {
  char *at = strstr(line, "mapname");
  if (at) {
    sscanf(at, "mapname\\%64[^\\]", game->mapname);
  }

  at = strstr(line, "g_gametype");
  if (at) {
    sscanf(at, "g_gametype\\%d", &game->gametype);
  }

  game->player_scores = 0;

  debug_info(DBGLVL + 1, "init game: host %s map %s type %d\n", 
    game->hostname, game->mapname, game->gametype);
}

void init_legacy_game(const char* line, GameScore *game) {
  sscanf(line, ": %64s", game->mapname);
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
      game->players[i].guid[0] = '#';
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
      game->client_connect = i;
      return;
    }
    if (*slot_idx == idx * -1) {
      game->client_connect = i;
      *slot_idx = idx;
      return;
    }
    if (game->players[i].idx == Empty && empty == -1) {
      empty = i;
    }
  }
  Player *pl = &game->players[empty];
  pl->idx = idx;
  pl->name[0] = '\0';
  pl->guid[0] = '#'; //in case of cli batch import
  pl->guid[1] = '\0';
  game->client_connect = empty;
  debug_info(DBGLVL, "Player: idx %d new\n", game->players[empty].idx);
}

void player_begin(const char* line, GameScore *game) {
  game->client_connect = -1;
}

void player_info(const char* line, GameScore *game) {
  if (game->client_connect == -1 || game->client_connect > 20) {
    return;
  }
  static char name[64], guid[33];

  Player *pl = game->players + game->client_connect;
  game->client_connect = -1;

  char *at = strstr(line, "cl_guid");
  if (at) {
    sscanf(at, "cl_guid\\%33[^\\]", guid);
  }
  else {
    return;
  }
  at = strstr(line, "name");
  if (at) {
    sscanf(at, "name\\%64[^\\]", name);
  }
  else {
    return;
  }
  if (strcmp(name, pl->name) != 0
    || strcmp(guid, pl->guid) != 0) {
    strncpy(pl->name, name, 64);
    strncpy(pl->guid, guid, 33);
    data_sync_player(pl, true);
  }
}

void player_info_change(const char* line, GameScore *game) {
  static char name[64];
  int idx, team;

  sscanf(line, ": %d n\\%64[^\\]\\t\\%d\\c", &idx, name, &team);

  int found = -1;
  for(int i = 0; i < MAX_PLAYERS; i++) {
    if (game->players[i].idx == idx) {
      found = i;
      game->players[i].team = team;
      if (strncmp(game->players[i].name, name, 64) != 0) {
        strncpy(game->players[i].name, name, 64);
        data_sync_player(game->players + i, false);
      }
      break;
    }
  }
  if (found > -1) {
    debug_info(DBGLVL + 1, "Player: name %s idx %d team %d\n", 
      game->players[found].name, game->players[found].idx, game->players[found].team);
  }
  else {
    debug_info(DBGLVL, "Player not found: name %s idx %d team %d\n",
      name, idx, team);
  }
}

void team_score(const char* line, GameScore *game) {
  sscanf(line, ":%d blue%d", &game->team_red, &game->team_blue);
}

void team_legacy_score(const char* line, GameScore *game) {
  sscanf(line, ":%d allies:%d", &game->team_red, &game->team_blue);
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
    p->kills += kills;
    p->deaths += deaths;
    p->headshots += headshots;
  }
  if (p->kills > 0 || p->deaths >0) {
    game->player_scores++;
  }
  sscanf(buff, " %d %d", &p->damage_given, &p->damage_recieved);

  debug_info(DBGLVL, "player %d kills %d, deaths %d given %d received %d\n", 
    p->idx, p->kills, p->deaths, p->damage_given, p->damage_recieved);
}

void shutdown_game(const char* line, GameScore *game) {
  if (game->player_scores > 1) {
    save_game_scores(game);
  }

  Player *pl;
  for (int i=0; i < MAX_PLAYERS; i++) {
    pl = game->players + i;
    if (pl->idx < 0) {
      pl->idx = Empty;
    }
    if (pl->team == 3) {
      pl->kills = pl->deaths = pl->score =
      pl->damage_recieved = pl->damage_given = 0;
    }
  }
  debug_info(DBGLVL + 1, "game shutdown, scores %d\n", game->player_scores);
}

void tce_parse_init() {

  char *operations[ROUTES] = {
    [InitGame]="InitGame",
    [Map]="map",
    [ShutdownGame]="ShutdownGame",
    [ClientDisconnect]="ClientDisconnect",
    [ClientConnect]="ClientConnect",
    [ClientBegin]="ClientBegin",
    [Userinfo]="Userinfo",
    [ClientUserinfoChanged]="ClientUserinfoChanged",
    [Team]="red",
    [LegacyTeam]="axis",
    [WeaponStats]="WeaponStats",
    [Score]="score"
  };

  void (*parsers[ROUTES])(const char*, GameScore*) = {
    [InitGame]=init_game,
    [Map]=init_legacy_game,
    [ShutdownGame]=shutdown_game,
    [ClientDisconnect]=player_disconnect,
    [ClientConnect]=player_connect,
    [ClientBegin]=player_begin,
    [Userinfo]=player_info,
    [ClientUserinfoChanged]=player_info_change,
    [Team]=team_score,
    [LegacyTeam]=team_legacy_score,
    [WeaponStats]=weapons_stats,
    [Score]=player_score,
  };

  for (int i=0; i< ROUTES; i++) {
    router[i].op = operations[i];
    router[i].parse = parsers[i];
  }
}

void tce_parse_game_init(GameScore *game) {
  for(int i = 0; i < MAX_PLAYERS; i++) {
    game->players[i].idx = Empty;
    game->players[i].player_id = 0;
    game->players[i].guid[0] = '#';
  }
}

