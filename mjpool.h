#ifndef _MJPOOL_H
#define _MJPOOL_H

#include <stdbool.h>

struct mjPool {
    unsigned int    length;             // used length
    unsigned int    total;              // total length
    void**          data;               // point to resource
};
typedef struct mjPool*  mjPool;

extern void*    mjPool_Alloc( mjPool pool );
extern bool     mjPool_Free( mjPool pool, void* elem );

extern mjPool   mjPool_New();                   // create a new mjpool
extern bool     mjPool_Delete( mjPool pool );   // destory mjpool

#endif
