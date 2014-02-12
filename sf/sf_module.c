#include "sf_module.h"

static LIST_HEAD(sf_modules);

/*
===============================================================================
sf_modules_init
===============================================================================
*/
void 
sf_modules_init(void) {
  sf_module_t* mod;
  list_for_each_entry(mod, &sf_modules, node) {
    if (mod->init) mod->init();
  }
}

/*
===============================================================================
sf_modules_start
===============================================================================
*/
void 
sf_modules_start(void) {
  sf_module_t* mod;
  list_for_each_entry(mod, &sf_modules, node) {
    if (mod->start) mod->start();
  }
}

/*
===============================================================================
sf_register_module
===============================================================================
*/
void
sf_register_module(sf_module_t* mod) {
  if (!mod) return;
  list_add_tail(&mod->node, &sf_modules);
}
