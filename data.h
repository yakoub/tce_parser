#ifndef DATA
#define DATA

typedef enum {CLOSE, OPEN} stage;
void data_init(stage);

typedef struct {
  unsigned long player_id;
  char guid[33];
  int idx;
  int team;
  char name[64];
  int ping;
  int score;
  int kills;
  int deaths;
  int headshots;
  int damage_given;
  int damage_recieved;

} Player;

#define Empty -30

#define MAX_PLAYERS 20

typedef struct {
  char hostname[64];
  char mapname[64];
  int team_red;
  int team_blue;
  int player_scores;
  int gametype;
  unsigned long game_id;
  unsigned long server_id;

  Player players[MAX_PLAYERS];
  int client_connect;

} GameScore;

typedef enum {false = 0, true = 1} bool;

void save_game_scores(GameScore*);
void data_sync_player(Player*, bool guid_trust);
#endif
