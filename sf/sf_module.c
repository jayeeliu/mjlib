#include "sf_module.h"

static LIST_HEAD(sf_modules);

void sf_modules_init() {
  sf_module_t* mod;
  list_for_each_entry(mod, &sf_modules, node) {
    if (mod->init) mod->init();
  }
}

void
sf_register_module(sf_module_t* mod) {
  if (!mod) return;
  list_add_tail(&mod->node, &sf_modules);
}
