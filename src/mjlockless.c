#include "mjlockless.h"
#include "mjlog.h"
#include <stdlib.h>

/*
===============================================================================
mjlockless_push
===============================================================================
*/
void mjlockless_push(mjlockless lockless, void* value) {
  uint32_t tail = __sync_fetch_and_add(&lockless->tail, 1) % lockless->_size;
  lockless->_queue[tail] = value;
  __sync_synchronize();
}

/*
===============================================================================
mjlockless_pop
===============================================================================
*/
void* mjlockless_pop(mjlockless lockless) {
  uint32_t head = lockless->head;
  if (lockless->head % lockless->_size == lockless->tail % lockless->_size) {
    return NULL;
  }
  // get value, tail mybe add, but tail value not set
  void* value = lockless->_queue[head % lockless->_size];
  if (!value) return NULL;
  // compare and swap
  if (!__sync_bool_compare_and_swap(&lockless->head, head, head + 1)) {
    return NULL;
  }
  lockless->_queue[head % lockless->_size] = NULL;
  __sync_synchronize();
  return value;
}

/*
===============================================================================
mjlockless_new
  alloc mjlockless struct
===============================================================================
*/
mjlockless mjlockless_new(int size) {
  if (size <= 0) {
    MJLOG_ERR("size is not valid");
    return NULL;
  }
  mjlockless lockless = (mjlockless)calloc(1, sizeof(struct mjlockless) + 
      sizeof(void*) * size);
  if (!lockless) {
    MJLOG_ERR("calloc error");
    return NULL; 
  }
  lockless->_size = size;
  return lockless;
}

/*
===============================================================================
mjlockless_delete
  delete lockless struct
===============================================================================
*/
bool mjlockless_delete(mjlockless lockless) {
  if (!lockless) {
    MJLOG_ERR("lockless is null");
    return false;
  }
  free(lockless);
  return true;
}
