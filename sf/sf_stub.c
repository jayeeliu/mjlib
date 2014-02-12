#include "sf_stub.h"
#include "sf_module.h"
#include "sf_object.h"
#include "sf_worker.h"
#include "sf_timer.h"
#include <stdio.h>
#include <stdlib.h>

struct sf_stub_s {
  sf_object_HEAD
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
  sf_stub_t* obj = calloc(1, sizeof(sf_stub_t));
  if (!obj) {
    free(obj);
    return;
  }
  sf_object_INIT(obj, NULL)
  obj->handler = stub_work;

  obj->timer1 = sf_timer_create((sf_object_t*)obj);
  if (!obj->timer1) return;
  sf_timer_enable(obj->timer1, 1000);

  obj->timer2 = sf_timer_create((sf_object_t*)obj);
  if(!obj->timer2) return;
  sf_timer_enable(obj->timer2, 2000);
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
