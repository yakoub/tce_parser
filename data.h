#ifndef DATA
#define DATA
typedef struct _data_config {
  const char* user;
  const char* password;
  const char* host;
  const char* db;
} DataConfig;

void config_data(DataConfig*);

typedef enum {CLOSE, OPEN} stage;
void data_init(stage);

typedef struct {
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

  Player players[MAX_PLAYERS];
} GameScore;

void save_game(GameScore*);
#endif
