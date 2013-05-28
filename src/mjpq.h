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

struct mjPQ {
    struct pq_elt*  p;      // the element array
    unsigned int    used;   // used in array
    unsigned int    total;  // total size of array
};
typedef struct mjPQ* mjPQ;

extern long long    mjPQ_GetMinKey(mjPQ pq);
extern void*        mjPQ_GetMinValue(mjPQ pq);
extern bool         mjPQ_Insert(mjPQ pq, long long key, void* value);
extern void         mjPQ_DelMin(mjPQ pq);

extern mjPQ mjPQ_New();
extern bool mjPQ_Delete(mjPQ pq);

#endif
