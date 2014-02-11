#ifndef __SF_TIMER_H
#define __SF_TIMER_H

#include "sf_object.h"
#include "sf_rbtree.h"

struct sf_timer_s {
  unsigned long   expire;
  void*           data;
  struct rb_node  node;
  sf_object_t*    obj;
};
typedef struct sf_timer_s sf_timer_t;

extern sf_object_t*
sf_timer_create();

extern void
sf_timer_destory();

extern void
sf_timer_enable(sf_object_t*, unsigned long ms);

extern void
sf_timer_disbale(sf_object_t*);

#endif
