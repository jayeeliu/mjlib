#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjpq.h"

/** 
 * add more n elements to mjprioq
 * return: -1--failed, 0--success
 */
static int mjpq_readyplus(mjpq pq, unsigned int n)
{
    if (pq->p) { /* the queue is not empty */
        unsigned int i = pq->total; /* get size */
        n += pq->used; /* how many elements we wanted */
        if (n > i) { /* enough ?*/
            pq->total = 30 + n + (n >> 3); /* expand queue */
            struct pq_elt *tp = (struct pq_elt*) realloc(pq->p, pq->total * sizeof(struct pq_elt));  
            if (tp) { /* realloc success */
                pq->p = tp; /* we get new memory */
                return 0;
            }       
            pq->total = i; /* alloc failed, restore total number */
            return -1;
        }
        return 0; /* success return */
    }

    /* the queue is empty, initialize it */
    if (n < 16) n = 16; /* the init size of queue*/ 
    pq->used = 0; /* no used elements */
    pq->total = n; /* totoal element*/
    pq->p = (struct pq_elt*) malloc(n * sizeof(struct pq_elt)); /* alloc memory for the queue*/
    if (!pq->p) return -1;
    return 0;
}

/**
 * insert new element into prioq
 */
int mjpq_insert(mjpq pq, struct pq_elt *pe)
{
    if (mjpq_readyplus(pq, 1) < 0) return -1; /* expand array if needed */
    int j = pq->used++; /* we add a new element */
    while (j) {
        int i = (j - 1) / 2; /* cal the avg */
        if (pq->p[i].dt <= pe->dt) break; /* get the pos */
        pq->p[j] = pq->p[i]; /* swap */
        j = i;    
    }
    pq->p[j] = *pe; /* set the value */
    
    return 0;
}

/**
 *  return the min element in prioq
 */
int mjpq_min(mjpq pq, struct pq_elt *pe)
{
    if (!pq->p) return -1;
    if (!pq->used) return -1;
    *pe = pq->p[0];

    return 0;
}

void mjpq_delmin(mjpq pq)
{
    int i, j, n;
    
    if (!pq->p) return;
    n = pq->used;
    if (!n) return;
    i = 0;
    --n; /* recalucate size */
    for (;;) {
        j = i + i + 2;
        if (j > n) break;
        if (pq->p[j-1].dt <= pq->p[j].dt) --j; 
        if (pq->p[n].dt <= pq->p[j].dt) break;
        pq->p[i] = pq->p[j];
        i = j;
    } 
    pq->p[i] = pq->p[n];
    pq->used = n;
}

/**
 * create mjprioq struct
 */
mjpq mjpq_new()
{
    mjpq pq = (mjpq) calloc(1, sizeof(struct mjpq));
    if (!pq) return NULL;

    pq->p = NULL;
    pq->used = 0;
    pq->total = 0;

    return pq;
}

/**
 * delete a mjprioq
 */
void mjpq_delete(mjpq pq)
{
    if (!pq) return;
    free(pq->p);
    free(pq);
}
