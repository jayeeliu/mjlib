#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mjlog.h"
#include "mjmap.h"
#include "mjstr.h"

static mjitem mjitem_new(const char *key, mjStr value)
{
    mjitem item = ( mjitem ) calloc( 1, sizeof( struct mjitem ) );
    if ( !item ) {
        MJLOG_ERR( "mjitem calloc error" );
        return NULL;
    }

    /* set key and value */ 
    item->key   = mjStr_New();
    item->value = mjStr_New();
    if ( !item->key || !item->value ) {
        MJLOG_ERR("mjStr new error");
        mjStr_Delete( item->key );
        mjStr_Delete( item->value );
        free( item );
        return NULL;
    }
    mjStr_CopyS( item->key, ( char* )key );
    mjStr_Copy( item->value, value );
   
    /* init list */ 
    INIT_HLIST_NODE( &item->map_node );
    item->prev  = item->next = NULL;

    return item;
}

static void mjitem_delete( mjitem item )
{
    if ( !item ) {
        MJLOG_ERR( "item is null" );
        return;
    }

    /* free key */
    mjStr_Delete( item->key );
    mjStr_Delete( item->value );
    if ( item->prev ) {
        item->prev->next    = item->next;
    }
    if ( item->next ) {
        item->next->prev    = item->prev;
    }
    item->prev          = NULL;
    item->next          = NULL;

    /* free struct */
    free( item );
}

/**
 * gen hash value, copy from redis
 */
static unsigned int genhashvalue( const void* key, int len )
{
    uint32_t seed = 5381;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while (len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few 
       bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

static mjitem mjmap_search( mjmap map, const char* key )
{
    unsigned int hashvalue = genhashvalue( ( void* )key, strlen( key ) );
    unsigned int index = hashvalue % map->len;

    mjitem item = NULL;
    struct hlist_node *entry;
    
    hlist_for_each_entry(item, entry, &map->elem[index], map_node) { 
        if (strcmp(item->key->data, key) == 0) return item;
    }

    return NULL;
}

int mjMap_Add( mjmap map, const char* key, mjStr value )
{
    unsigned int hashvalue = genhashvalue( ( void* )key, strlen( key ) );
    unsigned int index = hashvalue % map->len;

    /* if we have the same key */   
    mjitem item = mjmap_search( map, key );
    if ( item ) return -1;
 
    /* generator a new mjitem */
    item = mjitem_new( key, value );
    if ( !item ) {
        MJLOG_ERR( "mjitem_new error" );
        return -1;
    }

    /* add to elem list */
    hlist_add_head( &item->map_node, &map->elem[index] );
    item->prev              = map->tail->prev;
    item->next              = map->tail;
    map->tail->prev->next   = item;
    map->tail->prev         = item;

    /* increse mjmap count */
    map->itemcount++;

    return 0;
}

/**
 * delete one element from mjmap
 */
int mjMap_Del( mjmap map, const char* key )
{
    mjitem item = mjmap_search( map, key );
    if ( !item ) {
        MJLOG_ERR( "mjMap_Del none" );
        return -1;
    }

    hlist_del( &item->map_node );
    mjitem_delete( item );

    map->itemcount--;
    return 0;
}

/*
===============================================
mjMap_Get
    get mjStr from key
===============================================
*/
mjStr mjMap_Get( mjmap map, const char* key )
{
    if ( !map || !key ) return NULL;
    
    mjitem item = mjmap_search( map, key );
    if ( !item ) return NULL;

    return item->value;
}

/*
==================================================
mjmap_GetNext
    
==================================================
*/
mjitem mjmap_GetNext( mjmap map, mjitem item )
{
    mjitem itemnext;
    if ( !item ) {
        itemnext = map->head->next;
    } else {
        itemnext = item->next;
    }

    if ( itemnext == map->tail ) return NULL;
    return itemnext;
}

/*
===============================================================
mjMap_New
    create new mjmap
===============================================================
*/
mjmap mjMap_New( int mapsize )
{
    /* create map struct */
    mjmap map = ( mjmap ) calloc( 1, sizeof( struct mjmap ) + 
                        mapsize * sizeof( struct hlist_node ) );
    if ( !map ) {
        MJLOG_ERR( "mjmap calloc error" );
        return NULL;
    }

    map->len        = mapsize;
    map->itemcount  = 0; 
    map->head       = mjitem_new( "head", NULL );
    map->tail       = mjitem_new( "tail", NULL );
    if ( !map->head || !map->tail ) {
        MJLOG_ERR( "mjitem_new error" );
        mjitem_delete( map->head );
        mjitem_delete( map->tail );
        return NULL;    
    }
    map->head->prev  = NULL;
    map->head->next  = map->tail;
    map->tail->prev  = map->head;
    map->tail->next  = NULL;

    for ( int i = 0; i < mapsize; i++ ) {
        INIT_HLIST_HEAD( &map->elem[i] );
    }  

    return map;
}

void mjMap_Delete( mjmap map )
{
    if ( !map ) return;

    mjitem_delete( map->head );
    mjitem_delete( map->tail );
    for ( int i = 0; i < map->len; i++ ) {
        mjitem item = NULL;
        struct hlist_node *entry, *next;
        hlist_for_each_entry_safe( item, entry, next, 
                            &map->elem[i], map_node ) {
            hlist_del( &item->map_node );
            mjitem_delete( item );
        }
    }
    free( map );
}
