#include "sf_object.h"
#include <stdlib.h>

sf_object_t*
sf_object_create() {
  sf_object_t *obj = calloc(1, sizeof(sf_object_t));
  if (!obj) return NULL;
  INIT_LIST_HEAD(&obj->node);
  return obj;
}

void
sf_object_destory(sf_object_t* obj) {
  free(obj);
}
