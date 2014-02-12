#include "sf_epoll.h"
#include "sf_module.h"
#include "sf_util.h"
#include <stdio.h>
#include <pthread.h>
#include <sys/epoll.h>

#define MAXFD 60000

static int epfd;
static sf_epoll_t all_epoll[MAXFD];

/*
===============================================================================
sf_epoll_create
===============================================================================
*/
sf_epoll_t*
sf_epoll_create(unsigned fd, sf_object_t* parent) {
  if (fd >= MAXFD) return NULL;
  sf_epoll_t* epoll_t = &all_epoll[fd];
  epoll_t->parent = parent;
  return epoll_t;
}

/*
===============================================================================
sf_epoll_destory
===============================================================================
*/
void
sf_epoll_destory(sf_epoll_t* epoll_t) {
  sf_object_INIT(epoll_t, NULL);
  epoll_t->read_ready   = 0;
  epoll_t->write_ready  = 0;
}

static inline void
sf_init_epoll_event(struct epoll_event* ee, unsigned fd, unsigned mask) {
  ee->data.u64 = 0;
  ee->data.fd  = fd;
  ee->events   = 0;
  if (mask & SF_EPOLL_READ) ee->events |= EPOLLIN;
  if (mask & SF_EPOLL_WRITE) ee->events |= EPOLLOUT;
}

/*
===============================================================================
sf_epoll_enable
===============================================================================
*/
void
sf_epoll_enable(sf_epoll_t* epoll_t, unsigned mask) {
  unsigned new_mask = epoll_t->mask | mask;
  if (epoll_t->mask != new_mask) {
    struct epoll_event ee;
    sf_init_epoll_event(&ee, epoll_t->fd, new_mask);
    int op = (epoll_t->mask == SF_EPOLL_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    if (epoll_ctl(epfd, op, epoll_t->fd, &ee) == -1) return;
    epoll_t->mask = new_mask;
  }
}

/*
===============================================================================
sf_epoll_disable
===============================================================================
*/
void
sf_epoll_disable(sf_epoll_t* epoll_t, unsigned mask) {
  unsigned new_mask = epoll_t->mask | mask;
  if (epoll_t->mask != new_mask) {
    struct epoll_event ee;
    sf_init_epoll_event(&ee, epoll_t->fd, new_mask);
    int op = (new_mask == SF_EPOLL_NONE) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    if (epoll_ctl(epfd, op, epoll_t->fd, &ee) == -1) return;
    epoll_t->mask = new_mask;
  }
}

/*
===============================================================================
epoll_init
===============================================================================
*/
static void
epoll_init() {
  epfd = epoll_create(1024);
  for (int i=0; i<MAXFD; i++) {
    sf_epoll_t* epoll_t = &all_epoll[i];
    sf_object_INIT(epoll_t, NULL);
    epoll_t->fd = i;
  }
}

static void
epoll_do(sf_epoll_t* epoll_t, unsigned mask) {
  if (mask & SF_EPOLL_READ) {
    epoll_t->read_ready = 1;
  }
  if (mask & SF_EPOLL_WRITE) {
    epoll_t->write_ready = 1;
  }
  sf_object_notify((sf_object_t*)epoll_t);
}

static void*
epoll_routine(void* arg) {
  struct epoll_event epEvents[MAXFD];
  while (1) {
    int events_n = epoll_wait(epfd, epEvents, MAXFD, 300);
    printf("epoll_run\n");
    if (events_n <= 0) continue;
  }
  return NULL;
}

static void 
epoll_start() {
  pthread_t thread;
  pthread_create(&thread, NULL, epoll_routine, NULL);
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
