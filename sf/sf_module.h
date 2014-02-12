#ifndef __SF_MODULE_H
#define __SF_MODULE_H

#include "sf_list.h"

struct sf_module_s {
  void              (*init)(void);
  void              (*exit)(void);
  void              (*start)(void);
  void              (*end)(void);
  struct list_head  node;
};
typedef struct sf_module_s sf_module_t;

extern void 
sf_register_module(sf_module_t*);

extern void
sf_modules_init(void);

extern void
sf_modules_exit(void);

extern void
sf_modules_start(void);

extern void
sf_modules_end(void);

#endif
