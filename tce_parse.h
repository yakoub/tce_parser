#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "data.h"

#define BUFF_SIZE 2048 

void tce_parse(const char*, GameScore*);
void tce_parse_guid(const char*, GameScore*);
void tce_parse_game_init(GameScore *); 
