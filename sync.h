#include <stdio.h>
#include <string.h>
#include "conf.h"

void sync_logs_init();

void sync_logs_assign(int wd, const PathConfig*);

void sync_logs_close();

void sync_logs_rewind(const char* name, int wd);

void sync_logs(const char* name, int wd);
