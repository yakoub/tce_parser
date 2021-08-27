typedef struct {
  int idx;
  int team;
  char name[32];
  int ping;
  int score;
  int kills;
  int deaths;
  int headshots;

} Player;

#define MAX_PLAYERS 20

typedef struct {
  char hostname[32];
  char mapname[32];
  int team_red;
  int team_blue;
  int player_scores;

  Player players[MAX_PLAYERS];
} GameScore;

void save_game(GameScore*);
