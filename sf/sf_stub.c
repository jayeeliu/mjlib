#include "sf_stub.h"
#include "sf_module.h"
#include <stdio.h>

static void stub_init() {
  printf("stub init here\n");
}

sf_module_t stub_module = {
  stub_init,
  NULL,
  NULL,
  NULL,
};

__attribute__((constructor))
static void
__stub_init(void) {
  sf_register_module(&stub_module); 
}
