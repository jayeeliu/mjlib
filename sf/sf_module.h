#ifndef __SF_MODULE_H
#define __SF_MODULE_H

#include "sf_list.h"

struct sf_module_s {
  void              (*init)();
  void              (*exit)();
  void              (*start)();
  void              (*end)();
  struct list_head  node;
};
typedef struct sf_module_s sf_module_t;

extern void 
sf_register_module(sf_module_t*);

extern void
sf_modules_init();

extern void
sf_modules_start();

#endif
