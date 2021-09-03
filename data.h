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
#define CLOSE 1
#define OPEN 0
void data_init(char);
