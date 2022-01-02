#ifndef DEBUG
#define DEBUG

#include<stdarg.h>

#define DEBUG_METHOD 0

#if DEBUG_METHOD>0
  void debug_info(int level, const char* fmt, ...);
#else
#define debug_info(level, fmt, ...)
#endif

#endif

