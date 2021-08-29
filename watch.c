#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include "sync.h"
#include "debug.h"

void watch_handler(int ino_desc);
void report_read_error();

const char *paths[2] = {"./watch.d", "./watch2.d"};

int main(int argc, char* argv[]) {
  int ino_desc = inotify_init();  
  int ino_watch[2];

  sync_logs_init();
  for (int i=0; i<2; i++) {
    ino_watch[i] = inotify_add_watch(ino_desc, paths[i], IN_MODIFY | IN_CREATE);
    if (ino_watch[i] == -1) {
      printf("watch error %d", errno);
      return 1;
    }
    else {
      sync_logs_assign(ino_watch[i], paths[i]);
    }
  }

  watch_handler(ino_desc);
  for (int i=0; i<2; i++) {
    inotify_rm_watch(ino_watch[i], ino_desc);
  }
  close(ino_desc);
  sync_logs_close();

  debug_info(1, "close");

  return 0;
}

void watch_handler(int ino_desc) {
  char events_buff[4096]
       __attribute__ ((aligned(__alignof__(struct inotify_event))));
  ssize_t len;

  struct pollfd pfds[1];

  pfds[0].fd = ino_desc;
  pfds[0].events = POLLIN;
  

  int polling = 5;
  while(polling--) {
    printf("enter poll\n");
    poll(pfds, 1, -1);
    len=read(ino_desc, events_buff, sizeof(events_buff));

    if (len == -1) {
      report_read_error();
      return;
    }
    
    const struct inotify_event *ev = NULL;
    for (char *ptr = events_buff; ptr < events_buff + len;
         ptr += sizeof(struct inotify_event) + ev->len) {
      ev = (const struct inotify_event *) ptr;
      if (ev->mask & IN_CREATE) {
        sync_logs_rewind(ev->name, ev->wd);
      }
      sync_logs(ev->name, ev->wd);
    }
  }
}

void report_read_error() {
    printf("read error = %d\n", errno);
    int errno_s[] = {EAGAIN, EWOULDBLOCK, EBADF, EFAULT, EINTR, EINVAL, EIO, EISDIR};
    for (int i=0; i < 8; i++) {
      printf("\t %d", errno_s[i]);
    }
    printf("\n");
}

#if DEBUG_LEVEL>0
void debug_info(int level, const char* fmt, ...) {
  static FILE* fout = NULL;
  
  if (!fout) {
    fout = fopen("info.log", "w");
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

  if (level < DEBUG_LEVEL) {
    
    va_list args;
    va_start(args, fmt);
    vfprintf(fout, fmt, args);
    va_end(args);

    fflush(fout);
  }
}
#endif

