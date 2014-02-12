#include "sf_object.h"
#include "sf_module.h"
#include "sf_util.h"
#include "sf_worker.h"
#include <stdio.h>
#include <sys/epoll.h>

#define MAXFD 60000

static int epfd;
static struct epoll_event epEvents[MAXFD];

/*
===============================================================================
epoll_init
===============================================================================
*/
static void
epoll_init() {
  epfd = epoll_create(1024);
}

static void
epoll_worker(sf_object_t* obj) {
  static int delay = 0;
  if (delay > 300) delay = 300;
  int events_n = epoll_wait(epfd, epEvents, MAXFD, delay);
  if (events_n < 0) {
    //TODO:
  } else if (events_n == 0) {
    if (delay > 20) {
      delay *= 2;
    } else {
      delay += 5;
    }
  } else {
    delay = 0;
  }
  sf_worker_do(obj);
}

static void 
epoll_start() {
  sf_object_t* obj = sf_object_create(NULL);
  obj->handler = epoll_worker;
  sf_worker_do(obj);
}

sf_module_t epoll_module = {
  epoll_init,
  NULL,
  epoll_start,
  NULL,
};

__attribute__((constructor))
static void
__epoll_init(void) {
  sf_register_module(&epoll_module);
}
