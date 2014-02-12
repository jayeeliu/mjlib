#include "sf_object.h"
#include "sf_worker.h"
#include <stdlib.h>

/*
===============================================================================
sf_object_notify
===============================================================================
*/
void
sf_object_notify(sf_object_t* obj) {
  sf_worker_do(obj->parent);
}
