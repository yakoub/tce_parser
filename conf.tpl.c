#include "data.h"
#include <stddef.h>

char *paths[2] = {
  "./watch.d",
  "./watch2.d"
};

char** config_paths(int *count) {
  *count = 2;
  return paths;
}

void config_data(DataConfig *my) {
  my->user = "etl";
  my->password = NULL;
  my->host = "localhost";
  my->db = "tce_stats";
}
