#ifndef __SF_TIMER_H
#define __SF_TIMER_H

#include "sf_object.h"
#include "sf_rbtree.h"

struct sf_timer_s {
  sf_object_HEAD
  unsigned long     expire;
  struct rb_node    node;
  struct list_head  ready_node;
  unsigned          timeout:1;
  unsigned long     pad:63;
};
typedef struct sf_timer_s sf_timer_t;

extern sf_object_t*
sf_timer_create(sf_object_t* parent);

extern void
sf_timer_destory(sf_object_t* obj);

extern void
sf_timer_enable(sf_object_t* obj, unsigned long ms);

extern void
sf_timer_disbale(sf_object_t* obj);

#endif
