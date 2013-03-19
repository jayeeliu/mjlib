#include <stdlib.h>
#include <string.h>
#include "mjlog.h"
#include "mjpool.h"

#define MJPOOL_INIT_SIZE    1024
#define MJPOOL_MAX_SIZE     4096

static bool mjPool_Ready( mjPool pool, unsigned int n )
{
    if ( !pool ) return false;
    
    unsigned int i = pool->total;
    if ( n <= i ) return true;

    pool->total = 30 + n + ( n >> 3 );

    void** tmp = realloc( pool->data,
                pool->total * sizeof( void* ) );
    if ( !tmp ) {
        MJLOG_ERR( "realloc error" );
        pool->total = i;
        return false;
    }
    pool->data = tmp;

    for( int i = pool->length; i < pool->total; i++ ) {
        pool->data[i] = 0;
    }
    return true;
}

static bool mjPool_ReadyPlus( mjPool pool, unsigned int n )
{
    return mjPool_Ready( pool, pool->length + 1 );
}

void* mjPool_Alloc( mjPool pool )
{
    // santy check
    if ( !pool || !pool->data ) return NULL;
    // alloc memory from pool
    void* elem = NULL;
    if ( pool->length > 0 ) elem = pool->data[--pool->length];

    return elem;
}

bool mjPool_Free( mjPool pool, void* elem ) 
{
    // santy check
    if ( !pool || !pool->data || !elem ) return false;

    if ( !mjPool_ReadyPlus( pool, 1 ) ) {
        MJLOG_ERR( "mjPool_ReadyPlus Error" );
        return false;
    }

    pool->data[pool->length++] = elem;

    return true;
}


mjPool mjPool_New()
{
    // alloc resource pool struct
    mjPool pool = ( mjPool ) calloc ( 1, sizeof( struct mjPool ));
    if ( !pool ) {
        MJLOG_ERR( "mjpool alloc error" );
        return NULL;
    }
    // alloc element
    pool->data = calloc( 1, MJPOOL_INIT_SIZE * sizeof( void* ) );
    if ( !pool->data ) {
        MJLOG_ERR( "mjpool element alloc error" );
        free( pool );
        return NULL;
    }
    // set initial value
    pool->total     = MJPOOL_INIT_SIZE;
    pool->length    = 0;

    return pool;
}

bool mjPool_Delete( mjPool pool )
{
    // santy check
    if ( !pool ) return false;
    // free item
    free( pool->data );
    // free struct
    free( pool );
    return true;
}
