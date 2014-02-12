#ifndef __SF_OBJECT_H
#define __SF_OBJECT_H

#include "sf_list.h"

struct sf_object_s {
  void                (*handler)(struct sf_object_s*);
  void*               ctx;
  struct sf_object_s* parent;
  struct list_head    worker_node;
};
typedef struct sf_object_s sf_object_t;

extern void
sf_object_notify(sf_object_t* obj);

extern sf_object_t*
sf_object_create(sf_object_t* parent);

extern void
sf_object_destory(sf_object_t* obj);

#endif
