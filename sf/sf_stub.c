#include "sf_stub.h"
#include "sf_module.h"
#include "sf_object.h"
#include "sf_worker.h"
#include "sf_timer.h"
#include <stdio.h>
#include <stdlib.h>

struct sf_stub_s {
  sf_object_t*  timer1;
  sf_object_t*  timer2;
};
typedef struct sf_stub_s sf_stub_t;

static void
stub_work(sf_object_t* obj) {
  printf("stub work here\n");
}

static void
stub_start() {
  sf_object_t* obj = sf_object_create(NULL);
  if (!obj) return;
  obj->handler = stub_work;

  sf_object_t* obj1 = sf_timer_create(obj);
  if (!obj1) return;
  sf_timer_enable(obj1, 1000);

  sf_object_t* obj2 = sf_timer_create(obj);
  if(!obj2) return;
  sf_timer_enable(obj2, 2000);
}

sf_module_t stub_module = {
  NULL,
  NULL,
  stub_start,
  NULL,
};

__attribute__((constructor))
static void
__stub_init(void) {
  sf_register_module(&stub_module); 
}
