#include "data.h"
#include <stddef.h>

#define PATHS 2
char *paths[PATHS] = {
  "./watch.d",
  "./watch2.d"
};

char** config_paths(int *count) {
  *count = PATHS;
  return paths;
}

void config_data(DataConfig *my) {
  my->user = "etl";
  my->password = NULL;
  my->host = "localhost";
  my->db = "tce_stats";
}
