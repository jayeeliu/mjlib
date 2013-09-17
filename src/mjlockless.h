#ifndef __MJLOCKLESS_H
#define __MJLOCKLESS_H

#include <stdint.h>
#include <stdbool.h>

struct mjlockless {
  uint32_t  _head;
  uint32_t  _tail;
  int       _size;
  void*     _queue[0];
};
typedef struct mjlockless* mjlockless;

extern void       mjlockless_push(mjlockless lockless, void* value);
extern void*      mjlockless_pop(mjlockless lockless);
extern mjlockless mjlockless_new(int size);
extern bool       mjlockless_delete(mjlockless lockless);

#endif
