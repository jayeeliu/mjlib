#include "sf_stub.h"
#include "sf_module.h"
#include "sf_object.h"
#include "sf_worker.h"
#include <stdio.h>
#include <stdlib.h>

static void*
stub_work(sf_object_t* obj) {
  printf("stub work here\n");
  sf_object_destory(obj);
  return NULL;
}

static void 
stub_init() {
  printf("stub init here\n");
}

static void
stub_start() {
  sf_object_t* obj = sf_object_create();
  if (!obj) return;
  obj->handler = stub_work;
  sf_worker_enqueue(obj);
}

sf_module_t stub_module = {
  stub_init,
  NULL,
  stub_start,
  NULL,
};

__attribute__((constructor))
static void
__stub_init(void) {
  sf_register_module(&stub_module); 
}
