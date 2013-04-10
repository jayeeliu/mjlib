#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjpq.h"

/*
======================================================
mjPQ_ReadyPlus
    add more n elements to mjprioq
    return: -1--failed, 0--success
======================================================
*/
static int mjPQ_ReadyPlus( mjPQ pq, unsigned int n )
{
    if ( pq->p ) {  // the queue is not empty
        unsigned int i = pq->total; // get size
        n += pq->used; // how many elements we wanted
        if ( n > i ) { // not enough ?
            pq->total = 30 + n + (n >> 3); // expand queue
            struct pq_elt *tp = ( struct pq_elt* ) realloc( pq->p, pq->total * sizeof( struct pq_elt ) );  
            if ( tp ) { // realloc success 
                pq->p = tp; // we get new memory
                return 0;
            }       
            pq->total = i; // alloc failed, restore total number
            return -1;
        }
        return 0; // success return
    }

    // the queue is empty, initialize it
    if ( n < 16 ) n = 16; // the init size of queue
    pq->used = 0;   // no used elements
    pq->total = n;  // totoal element
    // alloc memory for the queue
    pq->p = ( struct pq_elt* ) malloc( n * sizeof( struct pq_elt ) );
    if ( !pq->p ) return -1;
    return 0;
}

/*
=====================================================
mjPQ_Insert
    insert new element into prioq
=====================================================
 */
bool mjPQ_Insert( mjPQ pq, long long key, void* value )
{
    // expand array if needed
    if ( mjPQ_ReadyPlus( pq, 1 ) < 0 ) return false;
    // we add a new element
    int j = pq->used++;
    while ( j ) {
        // caculate the average
        int i = ( j - 1 ) / 2;
        // get the position
        if ( pq->p[i].key <= key ) break;
        // swap
        pq->p[j] = pq->p[i];
        j = i;    
    }
    //set the value
    pq->p[j].key     = key;
    pq->p[j].value   = value; 
    
    return true;
}

/*
========================================
mjPQ_GetMinKey
    return the min element in prioq
    if no element, return -1
========================================
*/
long long mjPQ_GetMinKey( mjPQ pq )
{
    if ( !pq->p ) return -1;
    if ( !pq->used ) return -1;

    return pq->p[0].key;
}

/*
=====================================
mjPQ_GetMinValue
    get the min value in prioq
    if no element return NULL
=====================================
*/
void* mjPQ_GetMinValue( mjPQ pq )
{
    if ( !pq->p ) return NULL;
    if ( !pq->used ) return NULL;

    return pq->p[0].value;
}

/*
====================================
mjPQ_DelMin
    delete min element
    no return
====================================
*/
void mjPQ_DelMin( mjPQ pq )
{
    if ( !pq->p ) return;

    int n = pq->used;
    if ( !n ) return;

    int i = 0;
    int j;
    --n; // recalucate size
    for ( ;; ) {
        j = i + i + 2;
        if ( j > n ) break;
        if ( pq->p[j-1].key <= pq->p[j].key ) --j; 
        if ( pq->p[n].key <= pq->p[j].key ) break;
        pq->p[i] = pq->p[j];
        i = j;
    } 
    pq->p[i] = pq->p[n];
    pq->used = n;
}

/*
========================================================
mjPQ_New
    create mjPQ struct
========================================================
*/
mjPQ mjPQ_New()
{
    mjPQ pq = ( mjPQ ) calloc( 1, sizeof( struct mjPQ ) );
    if ( !pq ) return NULL;

    pq->p       = NULL;
    pq->used    = 0;
    pq->total   = 0;

    return pq;
}

/*
===========================
mjPQ_Delete
    delete a mjpq
===========================
*/
void mjPQ_Delete( mjPQ pq )
{
    if ( !pq ) return;
    free( pq->p );
    free( pq );
}
