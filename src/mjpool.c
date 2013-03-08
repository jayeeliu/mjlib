#include <stdlib.h>
#include <string.h>
#include "mjpool.h"

#define MJPOOL_INIT_SIZE 1024
#define MJPOOL_MAX_SIZE 4096

/* alloc new element from mjpool */
void* mjpool_alloc(mjpool p)
{
    /* santy check */
    if (!p || !p->pool) return NULL;
   
    /* alloc memory from pool */
    void *e = NULL;
    if (p->length > 0) e = p->pool[--p->length];

    return e;
}

/**
 * put element to mjpool 
 * return: success--true, fail--false 
 */
bool mjpool_free(mjpool p, void *e) 
{

    /* santy check */
    if (!p || !p->pool || !e) return false;

    if (p->length >=  p->total) {
        /* too large can't alloc */
        if (p->total >= MJPOOL_MAX_SIZE) {
            return false;
        }

        /* 列表空间不足够，重新分配 */
        int newsize = p->total * 2;
        if (newsize > MJPOOL_MAX_SIZE) newsize = MJPOOL_MAX_SIZE;

        /* 重新分配空间 */ 
        void *new_pool = realloc(p->pool, newsize * sizeof(void*));
        if (!new_pool) return false;
        
        /* 重置列表的各项 */
        p->total = newsize;
        p->pool = new_pool;
    }
    p->pool[p->length++] = e;

    return true;
}

/* create a new memory pool */
mjpool mjpool_new()
{
    /* alloc memory pool struct */
    mjpool p = (mjpool) calloc(1, sizeof(struct mjpool));
    if (!p) return NULL;

    /* alloc item size */
    p->pool = calloc(1, MJPOOL_INIT_SIZE * sizeof(void*));
    if (!p->pool) {
        free(p);
        return NULL;
    }

    /* set initial value */
    p->total = MJPOOL_INIT_SIZE;
    p->length = 0;

    return p;
}

/* delete a memory pool */
void mjpool_delete(mjpool p)
{
    /* santy check */
    if (!p) return;

    /* free item */
    free(p->pool);

    /* free struct */
    free(p);
}
