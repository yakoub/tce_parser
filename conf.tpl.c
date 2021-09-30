#include "data.h"
#include "conf.h"
#include <stddef.h>

#define PATHS 2

PathConfig path_conf[PATHS] = {
  {
    .path = "./watch.d", 
    .hostname = "^3first^rHost", 
    .ip = "127.0.0.1", 
    .port = 27960
  },
  {
    .path = "./watch2.d", 
    .hostname = "^?second^8Host", 
    .ip = "127.0.0.2", 
    .port = 27920
  }
};

PathConfig* config_paths(int *count) {
  *count = PATHS;
  return path_conf;
}

void config_data(DataConfig *my) {
  my->user = "etl";
  my->password = NULL;
  my->host = "localhost";
  my->db = "tce_stats";
}
