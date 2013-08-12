#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjpq.h"

/*
===============================================================================
mjpq_ReadyPlus
  add more n elements to mjprioq
  return: -1--failed, 0--success
===============================================================================
*/
static int mjpq_readyplus(mjpq pq, unsigned int n) {
  // the queue is not empty
  if (pq->p) {
    // get size
    unsigned int i = pq->total;
    // how many elements we wanted
    n += pq->used;
    if (n > i) { 
      // not enough, expand queeu
      pq->total = 30 + n + (n >> 3);
      struct pq_elt *tp = (struct pq_elt*) realloc(pq->p, 
              pq->total * sizeof(struct pq_elt));  
      // realloc success
      if (tp) {
        pq->p = tp;
        return 0;
      }
      // alloc failed, restore total number 
      pq->total = i;
      return -1;
    }
    return 0; // success return
  }
  // the queue is empty, initialize it
  if (n < 16) n = 16;
  // no used elements
  pq->used = 0;
  // total elements
  pq->total = n;
  // alloc memory for the queue
  pq->p = (struct pq_elt*) malloc(n * sizeof(struct pq_elt));
  if (!pq->p) return -1;
  return 0;
}

/*
===============================================================================
mjpq_Insert
  insert new element into prioq
===============================================================================
 */
bool mjpq_insert(mjpq pq, long long key, void* value) {
  // expand array if needed
  if (mjpq_readyplus(pq, 1) < 0) return false;
  // we add a new element
  int j = pq->used++;
  while (j) {
    // caculate the average
    int i = (j - 1) / 2;
    // get the position
    if (pq->p[i].key <= key) break;
    // swap
    pq->p[j] = pq->p[i];
    j = i;  
  }
  //set the value
  pq->p[j].key    = key;
  pq->p[j].value  = value; 
  return true;
}

/*
===============================================================================
mjpq_GetMinKey
  return the min element in prioq
  if no element, return -1
===============================================================================
*/
long long mjpq_get_minkey(mjpq pq) {
  if (!pq->p) return -1;
  if (!pq->used) return -1;
  return pq->p[0].key;
}

/*
===============================================================================
mjpq_GetMinValue
  get the min value in prioq
  if no element return NULL
===============================================================================
*/
void* mjpq_get_minvalue(mjpq pq) {
  if (!pq->p) return NULL;
  if (!pq->used) return NULL;
  return pq->p[0].value;
}

/*
====================================
mjpq_DelMin
  delete min element
  no return
====================================
*/
void mjpq_delmin(mjpq pq) {
  if (!pq->p) return;

  int n = pq->used;
  if (!n) return;

  int i = 0;
  int j;
  --n; // recalucate size
  for (;;) {
    j = i + i + 2;
    if (j > n) break;
    if (pq->p[j-1].key <= pq->p[j].key) --j; 
    if (pq->p[n].key <= pq->p[j].key) break;
    pq->p[i] = pq->p[j];
    i = j;
  } 
  pq->p[i] = pq->p[n];
  pq->used = n;
}

/*
===============================================================================
mjpq_New
  create mjpq struct
===============================================================================
*/
mjpq mjpq_new() {
  // alloc mjpq struct
  mjpq pq = (mjpq) calloc(1, sizeof(struct mjpq));
  if (!pq) return NULL;
  return pq;
}

/*
===============================================================================
mjpq_Delete
  delete a mjpq
===============================================================================
*/
bool mjpq_delete(mjpq pq) {
  if (!pq) return false;
  free(pq->p);
  free(pq);
  return true;
}
