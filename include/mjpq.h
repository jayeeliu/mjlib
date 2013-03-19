#ifndef _MJPQ_H
#define _MJPQ_H

/**
 * the data hold in prioq
 * dt is used for sorting
 * data is a pointer to the data
 */
struct pq_elt {
    long long dt;
    void *data;
};

struct mjpq {
    struct pq_elt *p;   /* the element array */
    unsigned int used;  /* used in array */
    unsigned int total; /* total size of array */
};

typedef struct mjpq* mjpq;

extern int mjpq_insert(mjpq pq, struct pq_elt *pe);
extern int mjpq_min(mjpq pq, struct pq_elt *pe);
extern void mjpq_delmin(mjpq pq);

extern mjpq mjpq_new();
extern void mjpq_delete(mjpq pq);

#endif
