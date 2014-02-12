#ifndef __SF_OBJECT_H
#define __SF_OBJECT_H

#include "sf_list.h"

#define sf_object_HEAD                                    \
  void                (*handler)(struct sf_object_s*);    \
  struct sf_object_s* parent;                             \
  struct list_head    worker_node;

#define sf_object_INIT(obj, pobj)                         \
  INIT_LIST_HEAD(&obj->worker_node);                      \
  obj->parent = pobj;

struct sf_object_s {
  sf_object_HEAD
};
typedef struct sf_object_s sf_object_t;

extern void
sf_worker_do(sf_object_t* obj);

static inline void
sf_object_notify(sf_object_t* obj) {
  sf_worker_do(obj->parent);
}

#endif
