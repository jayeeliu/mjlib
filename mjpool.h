#ifndef _MJPOOL_H
#define _MJPOOL_H

#include <stdbool.h>

struct mjpool {
    int total;              /* the length of memory pool */
    int length;             /* <length has been used */
    void **pool;            /* pool item */
};

typedef struct mjpool*  mjpool;

extern void* mjpool_alloc(mjpool p);
extern bool mjpool_free(mjpool p, void *e);

extern mjpool mjpool_new();             /* create a new mjpool */
extern void mjpool_delete(mjpool p);   /* destory mjpool */

#endif
