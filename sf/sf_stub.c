#include "sf_stub.h"
#include "sf_module.h"
#include "sf_object.h"
#include "sf_worker.h"
#include "sf_timer.h"
#include <stdio.h>
#include <stdlib.h>

static void*
stub_work1(sf_object_t* obj) {
  sf_timer_t* timer = obj->ctx;
  printf("stub work1 here:%lu\n", timer->expire);
  sf_timer_enable(obj, 1000);
  return NULL;
}

static void*
stub_work2(sf_object_t* obj) {
  sf_timer_t* timer = obj->ctx;
  printf("stub work2 here:%lu\n", timer->expire);
  sf_timer_enable(obj, 2000);
  return NULL;
}

static void 
stub_init() {
  printf("stub init here\n");
}

static void
stub_start() {
  /*
  sf_object_t* obj1 = sf_timer_create();
  if (!obj1) return;
  obj1->handler = stub_work1;
  sf_timer_enable(obj1, 1000);

  sf_object_t* obj2 = sf_timer_create();
  if(!obj2) return;
  obj2->handler = stub_work2;
  sf_timer_enable(obj2, 2000);
  */
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
