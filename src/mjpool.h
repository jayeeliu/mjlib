#ifndef _MJPOOL_H
#define _MJPOOL_H

#include "mjlockless.h"

struct mjpool {
    unsigned int    _total;              // total length
    uint32_t        _length; 
    mjlockless      _data; 
};
typedef struct mjpool*  mjpool;

extern void*    mjpool_alloc(mjpool pool);
extern bool     mjpool_free(mjpool pool, void* value);
extern mjpool   mjpool_new(int total);                    // create a new mjpool
extern bool     mjpool_delete(mjpool pool);               // destory mjpool

#endif
