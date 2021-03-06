#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include "sync.h"
#include "debug.h"
#include "data.h"
#include "conf.h"

#define DBGLVL 4

void watch_handler(int ino_desc);

int main(int argc, char* argv[]) {
  int ino_desc = inotify_init();  
  int path_count;
  PathConfig *conf = config_paths(&path_count);
  int *ino_watch = malloc(path_count * sizeof(int));

  sync_logs_init();

  for (int i=0; i < path_count; i++) {
    ino_watch[i] = inotify_add_watch(ino_desc, conf[i].path, IN_MODIFY | IN_CREATE);
    if (ino_watch[i] == -1) {
      printf("watch error %d, path %s", errno, conf[i].path);
      return 1;
    }
    else {
      sync_logs_assign(ino_watch[i], conf + i);
    }
  }

  watch_handler(ino_desc);

  for (int i=0; i < path_count; i++) {
    inotify_rm_watch(ino_watch[i], ino_desc);
  }
  close(ino_desc);
  sync_logs_close();

  debug_info(1, "close");
  data_init(CLOSE);

  return 0;
}

void watch_handler(int ino_desc) {
  char events_buff[4096]
       __attribute__ ((aligned(__alignof__(struct inotify_event))));
  struct pollfd pfds[1];

  pfds[0].fd = ino_desc;
  pfds[0].events = POLLIN;
  
  ssize_t len;
  while(1) {
    debug_info(1, "enter poll\n");
    poll(pfds, 1, -1);
    len=read(ino_desc, events_buff, sizeof(events_buff));
    
    const struct inotify_event *ev = NULL;
    for (char *ptr = events_buff; ptr < events_buff + len;
         ptr += sizeof(struct inotify_event) + ev->len) {
      ev = (const struct inotify_event *) ptr;
      if (ev->mask & IN_CREATE) {
        debug_info(DBGLVL, "rewind for %s wd = %d\n", ev->name, ev->wd);
        sync_logs_rewind(ev->name, ev->wd);
      }
      sync_logs(ev->name, ev->wd);
    }
  }
}

