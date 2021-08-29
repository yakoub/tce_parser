#ifndef DEBUG
#define DEBUG

#include<stdarg.h>

#define DEBUG_LEVEL 0

#if DEBUG_LEVEL>0
  void debug_info(int level, const char* fmt, ...);
#else
  #define debug_info(level, fmt, ...)
#endif

#endif

