#ifndef __MJLOCKLESS_H
#define __MJLOCKLESS_H

struct mjlockless {
  uint32_t  head;
  uint32_t  tail;
  int       _length;
  void*     _queue[0];
};
typedef struct mjlockless* mjlockless;

extern mjlockless mjlockless_new(int size);
extern bool       mjlockless_delete(mjlockless lockless);

#endif
