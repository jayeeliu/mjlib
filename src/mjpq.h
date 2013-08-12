#ifndef _MJPQ_H
#define _MJPQ_H

#include <stdbool.h>

/*
 * the data hold in prioq
 * key is used for sorting
 * value is a pointer to the data
 */
struct pq_elt {
    long long   key;
    void*       value;
};

struct mjpq {
    struct pq_elt*  p;      // the element array
    unsigned int    used;   // used in array
    unsigned int    total;  // total size of array
};
typedef struct mjpq* mjpq;

extern long long    mjpq_get_minkey(mjpq pq);
extern void*        mjpq_get_minvalue(mjpq pq);
extern bool         mjpq_insert(mjpq pq, long long key, void* value);
extern void         mjpq_delmin(mjpq pq);

extern mjpq mjpq_new();
extern bool mjpq_delete(mjpq pq);

#endif
