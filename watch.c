#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include "sync.h"

void watch_handler(int ino_desc, int ino_watch);
void report_read_error();

int main(int argc, char* argv[]) {
  int ino_desc = inotify_init();  

  int ino_watch = inotify_add_watch(ino_desc, "./watch.d", IN_MODIFY | IN_CREATE);
  if (ino_watch == -1) {
    printf("watch error %d", errno);
    return 1;
  }

  sync_logs_init(); 
  watch_handler(ino_desc, ino_watch);

  inotify_rm_watch(ino_watch, ino_desc);
  close(ino_desc);
  sync_logs_close();

  return 0;
}

void watch_handler(int ino_desc, int ino_watch) {
  char events_buff[4096]
       __attribute__ ((aligned(__alignof__(struct inotify_event))));
  ssize_t len;

  struct pollfd pfds[1];

  pfds[0].fd = ino_desc;
  pfds[0].events = POLLIN;
  

  int polling = 1;
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
        sync_logs_rewind(ev->name);
      }
      sync_logs();
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
