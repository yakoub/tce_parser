#include <stdio.h>
#include <string.h>
#include "debug.h"

#if DEBUG_METHOD > 1
void debug_info(int level, const char* fmt, ...) {
  static FILE* fout = NULL;
  
  if (!fout) {
    fout = fopen("./watchinfo.log", "w");
    if (!fout) {
      fout = stdout;
    }
  }

  if (strcmp(fmt, "close") == 0) {
    if (fout != stdout) {
      fclose(fout);
    }
    return;
  }

  if (level < DEBUG_METHOD) {
    
    va_list args;
    va_start(args, fmt);
    vfprintf(fout, fmt, args);
    va_end(args);

    fflush(fout);
  }
}
#endif
#if DEBUG_METHOD == 1
#define MSGS_SIZE 5
void debug_info(int level, const char* fmt, ...) {
  static char msgs[MSGS_SIZE][1024];
  static int msgs_order[MSGS_SIZE];
  static char init = 0;
  static FILE* fout = NULL;

  if (!init) {
    init = 1;
    for (int i=0; i<MSGS_SIZE; i++) {
      msgs_order[i] = i;
      sprintf(msgs[i], "emtpty");
    }
  }
  
  if (level < 0) {
    fout = fopen("./livereport.log", "a");
    if (!fout) {
      fout = stdout;
    }
    for (int i=0; i < MSGS_SIZE; i++) {
      fprintf(fout, "%s\n", msgs[msgs_order[i]]);
    }
    fflush(fout);
    return;
  }

  if (level < DEBUG_METHOD) {
    for (int i=0; i<MSGS_SIZE; i++) {
      msgs_order[i]++;
      if (msgs_order[i]==5) {
        msgs_order[i]=0;
      }
    }
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgs[msgs_order[MSGS_SIZE-1]], 1023, fmt, args);
    va_end(args);
  }
}
#endif
