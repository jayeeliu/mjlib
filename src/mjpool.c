#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjpool.h"

void* mjpool_alloc(mjpool pool) {
  if (!pool || !pool->_length) return NULL;
  void* value = mjlockless_pop(pool->_data);
  if (value) {
    __sync_fetch_and_sub(&pool->_length, 1);
  }
  return value;
}

bool mjpool_free(mjpool pool, void* value) {
  // santy check
  if (!pool || !value) return false;
  uint32_t length = __sync_add_and_fetch(&pool->_length, 1);
  if (length > pool->_total) {
    __sync_sub_and_fetch(&pool->_length, 1);
    return false;
  }
  mjlockless_push(pool->_data, value);
  return true;
}

mjpool mjpool_new(int total) {
  // alloc resource pool struct
  mjpool pool = (mjpool) calloc (1, sizeof(struct mjpool));
  if (!pool) {
    MJLOG_ERR("mjpool alloc error");
    return NULL;
  }
  // alloc element
  pool->_data = mjlockless_new(total + 1);
  if (!pool->_data) {
    MJLOG_ERR("mjlockless_new error");
    free(pool);
    return NULL;
  }
  // set initial value
  pool->_total = total;
  return pool;
}

bool mjpool_delete(mjpool pool) {
  // santy check
  if (!pool) return false;
  // free item
  mjlockless_delete(pool->_data);
  // free struct
  free(pool);
  return true;
}
