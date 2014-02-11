#ifndef __SF_MODULE_H
#define __SF_MODULE_H

#include "sf_list.h"

struct sf_object_s {
  void*             (*handler)(struct sf_object_s*);
  void*             ctx;
  struct list_head  node;
};
typedef struct sf_object_s sf_object_t;

struct sf_module_s {
  void          (*init)();
  sf_object_t*  (*create)(void*);
  void          (*destroy)(sf_object_t*);
  void          (*exit)();
};
typedef struct sf_module_s sf_module_t;

extern void 
sf_register_module(sf_module_t*);

extern void 
sf_task_enqueue(sf_object_t*);

extern void
sf_create_worker();

#endif
