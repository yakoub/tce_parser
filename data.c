#include "data.h"
#include <stdio.h>

void save_game(GameScore *game) {
  printf("======================================================\n");
  printf("host %s map %s\n", game->hostname, game->mapname);
  printf("specop %d terrorist %d\n", game->team_blue, game->team_red);

  for (int i=0; i<20; i++) {
    if (game->players[i].idx == -1) {
      continue;
    }
    printf("team %d name %s ping %d kills %d\n", 
      game->players[i].team, 
      game->players[i].name, 
      game->players[i].ping,
      game->players[i].kills
    );
  }
}
