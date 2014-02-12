#include "sf_module.h"
#include "sf_timer.h"
#include "sf_util.h"
#include "sf_worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>

static struct rb_root timer_list;
static pthread_mutex_t timer_list_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
===============================================================================
sf_timer_do
===============================================================================
*/
static void
sf_timer_do(sf_object_t* obj) {
  sf_timer_t* timer = obj->ctx;
  timer->timeout = 1;
  sf_worker_do(obj->parent);
}

/*
===============================================================================
sf_timer_create
===============================================================================
*/
sf_object_t*
sf_timer_create(sf_object_t* parent) {
  sf_object_t* obj = sf_object_create(parent);
  if (!obj) return NULL;
  obj->handler = sf_timer_do;

  sf_timer_t* timer = calloc(1, sizeof(sf_timer_t));
  if (!timer) {
    free(obj);
    return NULL;
  }
  RB_CLEAR_NODE(&timer->node);
  INIT_LIST_HEAD(&timer->ready_node);
  timer->owner  = obj;

  obj->ctx      = timer;
  return obj;
}

/*
===============================================================================
sf_timer_destory
===============================================================================
*/
void
sf_timer_destory(sf_object_t* obj) {
  free(obj->ctx);
  free(obj);
}

/*
===============================================================================
sf_timer_enable
===============================================================================
*/
void
sf_timer_enable(sf_object_t* obj, unsigned long ms) {
  sf_timer_t* timer = obj->ctx;
  if (!RB_EMPTY_NODE(&timer->node)) return;

  timer->expire   = get_current_time() + ms;
  timer->timeout  = 0;

  pthread_mutex_lock(&timer_list_mutex);
  struct rb_node **new = &timer_list.rb_node, *parent = NULL;
  while (*new) {
    sf_timer_t* curr = rb_entry(*new, sf_timer_t, node);
    parent = *new;
    if (timer->expire < curr->expire) {
      new = &((*new)->rb_left);
    } else {
      new = &((*new)->rb_right);
    }
  }
  rb_link_node(&timer->node, parent, new);
  rb_insert_color(&timer->node, &timer_list);
  pthread_mutex_unlock(&timer_list_mutex);
}

/*
===============================================================================
sf_timer_disable
===============================================================================
*/
void
sf_timer_disable(sf_object_t* obj) {
  sf_timer_t* timer = obj->ctx;
  if (RB_EMPTY_NODE(&timer->node)) return;

  pthread_mutex_lock(&timer_list_mutex);
  rb_erase(&timer->node, &timer_list);
  pthread_mutex_unlock(&timer_list_mutex);
  rb_init_node(&timer->node);
}

static void* 
timer_routine(void* arg) {
  int epfd = epoll_create(1024);
  if (epfd < 0) return NULL;

  struct list_head ready_list;
  INIT_LIST_HEAD(&ready_list);

  while (1) {
    struct epoll_event epEvents[1];
    epoll_wait(epfd, epEvents, 1, 300);
    if (RB_EMPTY_ROOT(&timer_list)) continue;
    unsigned long curr_time = get_current_time();

    pthread_mutex_lock(&timer_list_mutex);
    if (RB_EMPTY_ROOT(&timer_list)) {
      pthread_mutex_unlock(&timer_list_mutex);
      continue;
    }        

    struct rb_node* first = rb_first(&timer_list);
    while (first) {
      // get timer task
      sf_timer_t* timer = rb_entry(first, sf_timer_t, node);
      if (timer->expire > curr_time) break;
      // put into ready_list
      rb_erase(&timer->node, &timer_list);
      rb_init_node(&timer->node);
      list_add_tail(&timer->ready_node, &ready_list);
      // try next
      first = rb_first(&timer_list);
    } 
    pthread_mutex_unlock(&timer_list_mutex);
    // enqueue ready task
    while (!list_empty(&ready_list)) {
      sf_timer_t* timer = list_first_entry(&ready_list, sf_timer_t, ready_node);
      list_del_init(&timer->ready_node);
      sf_worker_do(timer->owner);
    }
    INIT_LIST_HEAD(&ready_list);
  }
  return NULL;
}

/*
===============================================================================
timer_init
===============================================================================
*/
static void
timer_init(void) {
  timer_list = RB_ROOT;
}

/*
===============================================================================
timer_start
===============================================================================
*/
static void
timer_start(void) {
  pthread_t thread;
  pthread_create(&thread, NULL, timer_routine, NULL);
}

static sf_module_t timer_module = {
  timer_init,
  NULL,
  timer_start,
  NULL,
};

__attribute__((constructor))
static void
__timer_init(void) {
  sf_register_module(&timer_module);
}
