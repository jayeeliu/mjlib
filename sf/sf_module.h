#ifndef __SF_MODULE_H
#define __SF_MODULE_H

#include "sf_object.h"

struct sf_module_s {
  void              (*init)();
  sf_object_t*      (*create)(void*);
  void              (*destroy)(sf_object_t*);
  void              (*exit)();
  struct list_head  node;
};
typedef struct sf_module_s sf_module_t;

extern void 
sf_register_module(sf_module_t*);

extern void
sf_modules_init();

#endif
