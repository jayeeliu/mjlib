#ifndef __SF_OBJECT_H
#define __SF_OBJECT_H

#include "sf_list.h"

struct sf_object_s {
  void*             (*handler)(struct sf_object_s*);
  void*             ctx;
  struct list_head  node;
};
typedef struct sf_object_s sf_object_t;

extern void 
sf_object_enqueue(sf_object_t*);

extern void
sf_create_worker();

#endif
