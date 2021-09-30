#ifndef CONFIG
#define CONFIG

typedef struct {
  const char* path;
  const char* hostname;
  const char* ip;
  const int port;
} PathConfig;

PathConfig* config_paths(int *);

typedef struct _data_config {
  const char* user;
  const char* password;
  const char* host;
  const char* db;
} DataConfig;

void config_data(DataConfig*);


#endif
