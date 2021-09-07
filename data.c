#include "data.h"
#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include "debug.h"

#define DBGLVL 2

MYSQL *tce_db;

#define es(my, to, from, len) mysql_real_escape_string(my, to, from, len)

struct Sql {
  const char *insert_player;
  const char *insert_player_values;
  const char *insert_game;
} Query;

void save_game(GameScore *game) {

  data_init(OPEN);
  if (!tce_db) {
    debug_info(DBGLVL, "db connection failled\n");
    return;
  }

  char name1[128], name2[128], query_buff[5120];
  size_t written; 
  
  es(tce_db, name1, game->mapname, strlen(game->mapname));
  es(tce_db, name2, game->hostname, strlen(game->hostname));

  written = snprintf(query_buff, 5120, Query.insert_game,
    name1, name2, game->team_red, game->team_blue, game->gametype);
  if (written > 5120) {
    debug_info(DBGLVL, "aborted query %.256s\n", query_buff);
    return;
  }
  int ret = mysql_real_query(tce_db, query_buff, strlen(query_buff));
  if (ret != 0) {
    debug_info(DBGLVL, "failed : %s\n", query_buff);
    return;
  }
  //debug_info(DBGLVL, "%s\n", query_buff);
  long long int game_id = mysql_insert_id(tce_db);

  char player_buff[192];
  size_t total_written = strlen(Query.insert_player);
  Player *player;
  int idx;
  strcpy(query_buff, Query.insert_player);
  for (int i=0; i<MAX_PLAYERS; i++) {
    if (game->players[i].idx == Empty) {
      continue;
    }

    player = &game->players[i];
    es(tce_db, name1, player->name, strlen(player->name));

    if (player->idx < 0) {
      idx = player->idx * -1; 
      player->idx == Empty;
    }
    else {
      idx = player->idx; 
    }

    written = snprintf(player_buff, 192, Query.insert_player_values, 
      game_id, idx, player->team, name1, player->ping, player->score, 
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

void data_sql_init() {
  Query.insert_player = 
      "insert into game_player"
      " (match_id, idx, team, name, ping,"
      " score, kills, deaths, headshots, damage_given, damage_recieved)"
      " values ";
  Query.insert_player_values = "(%d, %d, %d, '%s', %d, %d, %d, %d, %d, %d, %d),";
  Query.insert_game = 
      "insert into game_match"
      " (mapname, hostname, team_red, team_blue, gametype)"
      " values ('%s', '%s', %d, %d, %d)";
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
