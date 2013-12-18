#ifndef _MJPOOL_H
#define _MJPOOL_H

#include "mjlockless.h"

struct mjpool {
  unsigned int  _total;     // total length
  uint32_t      _length;    // current used 
  unsigned int  _size;      // elem size
  mjlockless    _data; 
};
typedef struct mjpool*  mjpool;

extern void*    mjpool_alloc(mjpool pool);
extern bool     mjpool_free(mjpool pool, void* value);
extern mjpool   mjpool_new(unsigned int size, unsigned int total);
extern bool     mjpool_delete(mjpool pool);

#endif
