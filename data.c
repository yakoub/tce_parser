#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>
#include "debug.h"

#define DBGLVL 4

MYSQL *tce_db;

#define es(my, to, from, len) mysql_real_escape_string(my, to, from, len)

struct Sql {
  const char *insert_player_score;
  const char *insert_player_score_values;
  const char *insert_game;
  const char *fetch_player_name;
  const char *fetch_player_guid;
  const char *insert_player;
} Query;

void save_game(GameScore*, char* , char*, char*);
void save_player_scores(GameScore* , char*);

void save_game_scores(GameScore *game) {

  data_init(OPEN);
  if (!tce_db) {
    debug_info(DBGLVL, "db connection failled\n");
    return;
  }

  char name1[128], name2[128], query_buff[5120];
  
  save_game(game, name1, name2, query_buff);
  save_player_scores(game, query_buff);
}

void save_game(GameScore *game, char *name1, char *name2, char *query_buff) {
  size_t written; 

  es(tce_db, name1, game->mapname, strlen(game->mapname));
  es(tce_db, name2, game->hostname, strlen(game->hostname));

  written = snprintf(query_buff, 5120, Query.insert_game,
    name1, name2, game->team_red, game->team_blue, game->gametype);
  if (written > 5120) {
    debug_info(DBGLVL, "aborted query %.256s\n", query_buff);
    return;
  }
  int ret = mysql_query(tce_db, query_buff);
  if (ret != 0) {
    debug_info(DBGLVL, "failed : %s\n", query_buff);
    return;
  }
  debug_info(DBGLVL+1, "%.256s\n", query_buff);
  game->game_id = mysql_insert_id(tce_db);
}

void save_player_scores(GameScore *game, char *query_buff) {

  char player_buff[192];
  size_t total_written = strlen(Query.insert_player_score);
  Player *player;
  int idx;
  strcpy(query_buff, Query.insert_player_score);
  for (int i=0; i<MAX_PLAYERS; i++) {
    if (game->players[i].idx == Empty) {
      continue;
    }

    player = &game->players[i];

    if (player->idx < 0) {
      idx = player->idx * -1; 
      player->idx == Empty;
    }
    else {
      idx = player->idx; 
    }

    size_t written = snprintf(player_buff, 192, Query.insert_player_score_values, 
      game->game_id, idx, player->team, player->player_id, player->ping, player->score, 
      player->kills, player->deaths, player->headshots,
      player->damage_given, player->damage_recieved
    );

    if (written > 192) {
      debug_info(DBGLVL, "aborted query %.128s\n", query_buff);
      return;
    }
    total_written += written;
    if (total_written > 5120) {
      debug_info(DBGLVL, "aborted query %.256s\n", query_buff);
    }
    strncat(query_buff, player_buff, 192);
  }

  size_t last_comma = strlen(query_buff) - 1;
  query_buff[last_comma] = '\0';
  debug_info(DBGLVL + 1, "players data query %.512s\n", query_buff);
  int ret2 = mysql_real_query(tce_db, query_buff, last_comma);
  if (ret2 != 0) {
    debug_info(DBGLVL, "query failed : %s\n", query_buff);
  }
}

bool fetch_player(Player*, char*, char*, char*); 
void save_player(Player*, char*, char*, char*); 

void data_sync_player(Player* pl, bool guid_trust) {
  static char query_buff[256], name[64], guid[33];
  
  data_init(OPEN);

  es(tce_db, name, pl->name, strlen(pl->name));
  es(tce_db, guid, pl->guid, strlen(pl->guid));

  if (fetch_player(pl, query_buff, name, guid)) {
    return;
  }
  if (!guid_trust) {
    guid[0] = '#';
    guid[1] = '\0';
    pl->guid[0] = '#';
    pl->guid[1] = '\0';
  }
  save_player(pl, query_buff, name, guid);
}

bool fetch_player(Player* pl, char *query_buff, char *name, char *guid) {
  size_t written;

  if (guid[0] == '#') {
    written = snprintf(query_buff, 256, Query.fetch_player_name, name);
  }
  else {
    written = snprintf(query_buff, 256, Query.fetch_player_guid, name, guid);
  }

  if (written > 256) {
    debug_info(DBGLVL, "aborted query %.256s\n", query_buff);
    return false;
  }
  mysql_query(tce_db, query_buff);
  MYSQL_RES *result = mysql_store_result(tce_db);
  if (!result) {
    return false;
  }

  MYSQL_ROW row;
  row = mysql_fetch_row(result);
  if (!row) {
    return false;
  }
  pl->player_id = atol(row[0]);
  mysql_free_result(result);
  return true;
}

void save_player(Player* pl, char *query_buff, char *name, char *guid) {
  size_t written;

  written = snprintf(query_buff, 256, Query.insert_player, name, guid);

  if (written > 256) {
    debug_info(DBGLVL, "aborted query %.256s\n", query_buff);
    return;
  }
  int ret = mysql_query(tce_db, query_buff);
  if (ret != 0) {
    debug_info(DBGLVL, "failed : %s\n", query_buff);
    return;
  }
  pl->player_id = mysql_insert_id(tce_db);
}

void data_sql_init() {
  Query.insert_player_score = 
    "insert into game_player"
    " (match_id, idx, team, player_id, ping,"
    " score, kills, deaths, headshots, damage_given, damage_recieved)"
    " values ";
  Query.insert_player_score_values = "(%d, %d, %d, '%d', %d, %d, %d, %d, %d, %d, %d),";
  Query.insert_game = 
    "insert into game_match"
    " (mapname, hostname, team_red, team_blue, gametype)"
    " values ('%s', '%s', %d, %d, %d)";

  Query.fetch_player_guid = 
    "select id from player_index"
    " where name = '%s' and guid = '%s'";

  Query.fetch_player_name = 
    "select id from player_index"
    " where name = '%s'";

  Query.insert_player =
    "insert into player_index"
    " (name, guid)"
    " values ('%s', '%s')";
}

void data_init(stage st) {
  static char once = 0;

  if (tce_db != NULL) {
    return;
  }

  if (st == CLOSE) {
    debug_info(DBGLVL+1, "db closed\n");
    mysql_close(tce_db);
    return;
  }

  if (once) {
    debug_info(DBGLVL+1, "once return\n");
    return;
  }
  once = 1;

  tce_db = mysql_init(NULL);
  DataConfig my;
  config_data(&my);
  tce_db = mysql_real_connect(
    tce_db, my.host, my.user, my.password, my.db, 0, NULL, 0);
  if (tce_db) {
    debug_info(DBGLVL+1, "connection good\n");
  }
  else {
    debug_info(DBGLVL+1, "connection fail\n");
  }

  data_sql_init();
}
