#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mjlog.h"
#include "mjmap.h"
#include "mjstr.h"

/*
===============================================================================
mjItem_New
    create new mjItem struct
===============================================================================
*/
static mjItem mjItem_New( const char* key, const char* value ) {
    // alloc mjItem
    mjItem item = ( mjItem ) calloc ( 1, sizeof( struct mjItem ) );
    if ( !item ) {
        MJLOG_ERR( "mjItem calloc error" );
        return NULL;
    }
    // set key and value 
    item->key   = mjStr_New();
    item->value = mjStr_New();
    if ( !item->key || !item->value ) {
        MJLOG_ERR("mjStr new error");
        mjStr_Delete( item->key );
        mjStr_Delete( item->value );
        free( item );
        return NULL;
    }
    // set key and value
    mjStr_CopyS( item->key, key );
    mjStr_CopyS( item->value, value );
    // init list
    INIT_LIST_HEAD( &item->listNode );
    // init map list
    INIT_HLIST_NODE( &item->mapNode );
    //item->prev  = item->next = NULL;
    return item;
}

/*
===============================================================================
mjItem_Delete
    delete mjItem
===============================================================================
*/
static bool mjItem_Delete( mjItem item ) {
    // sanity check
    if ( !item ) {
        MJLOG_ERR( "item is null" );
        return false;
    }
    // free key
    mjStr_Delete( item->key );
    mjStr_Delete( item->value );
    // free struct
    free( item );
    return true;
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

/*
===============================================================================
mjMap_search
    search item
===============================================================================
*/
static mjItem mjMap_search( mjMap map, const char* key ) {
    // get hash value and index
    unsigned int hashvalue = genhashvalue( ( void* )key, strlen( key ) );
    unsigned int index = hashvalue % map->len;
    // search entry
    mjItem item = NULL;
    struct hlist_node *entry;
    hlist_for_each_entry( item, entry, &map->elem[index], mapNode ) { 
        if ( strcmp( item->key->data, key ) == 0 ) return item;
    }
    return NULL;
}

/*
===============================================================================
mjMap_Add
    add key and value to mjMap, call mjMap_AddS
    return  -1 --- error
            -2 --- already exists
             0 --- success
===============================================================================
*/
int mjMap_Add( mjMap map, const char* key, mjStr value ) {
    return mjMap_AddS( map, key, value->data );
}

/*
===============================================================================
mjMap_AddS
    add key and value to mjMap, call mjMap_AddS
    return  -1 --- error
            -2 --- already exists
             0 --- success
===============================================================================
*/
int mjMap_AddS( mjMap map, const char* key, const char* value ) {
    // get hash value and index
    unsigned int hashvalue = genhashvalue( ( void* )key, strlen( key ) );
    unsigned int index = hashvalue % map->len;
    // search entry
    mjItem item = mjMap_search( map, key );
    if ( item ) return -2;
    // generator a new mjItem
    item = mjItem_New( key, value );
    if ( !item ) {
        MJLOG_ERR( "mjItem_New error" );
        return -1;
    }
    // add to list
    list_add_tail( &item->listNode, &map->listHead );
    // add to elem list
    hlist_add_head( &item->mapNode, &map->elem[index] );
    return 0;
}

/*
===============================================================================
mjMap_Del
    delete one element from mjMap
===============================================================================
*/
int mjMap_Del( mjMap map, const char* key ) {
    mjItem item = mjMap_search( map, key );
    if ( !item ) {
        MJLOG_ERR( "mjMap_Del none" );
        return -1;
    }
    list_del( &item->listNode );
    hlist_del( &item->mapNode );
    mjItem_Delete( item );
    return 0;
}

/*
===============================================================================
mjMap_Get
    get mjStr from key
===============================================================================
*/
mjStr mjMap_Get( mjMap map, const char* key ) {
    // sanity check
    if ( !map || !key ) return NULL;
    // search mjtime 
    mjItem item = mjMap_search( map, key );
    if ( !item ) return NULL;
    return item->value;
}

/*
===============================================================================
mjMap_GetNext
    get next mjItem value 
===============================================================================
*/
mjItem mjMap_GetNext( mjMap map, mjItem item ) {
    // list is empty
    if ( list_empty( &map->listHead ) ) return NULL;
    if ( item == NULL ) {
        item = list_first_entry( &map->listHead, struct mjItem, listNode );
        return item;
    }
    // get next entry
    list_for_each_entry_continue( item, &map->listHead, listNode ) break;
    if ( &item->listNode == &map->listHead ) return NULL;
    return item;
}

/*
===============================================================================
mjMap_New
    create new mjMap
===============================================================================
*/
mjMap mjMap_New( int mapsize ) {
    // create map struct
    mjMap map = ( mjMap ) calloc( 1, sizeof( struct mjMap ) + 
                        mapsize * sizeof( struct hlist_node ) );
    if ( !map ) {
        MJLOG_ERR( "mjMap calloc error" );
        return NULL;
    }
    // set mjMap fields
    map->len        = mapsize;
    // init list
    INIT_LIST_HEAD( &map->listHead );
    // init hash list
    for ( int i = 0; i < mapsize; i++ ) INIT_HLIST_HEAD( &map->elem[i] );
    return map;
}

/*
===============================================================================
mjMap_Delete
    delete mjMap
===============================================================================
*/
bool mjMap_Delete( mjMap map ) {
    // sanity check
    if ( !map ) return false;
    // get and clean mjItem
    for ( int i = 0; i < map->len; i++ ) {
        mjItem item = NULL;
        struct hlist_node *entry, *next;
        hlist_for_each_entry_safe( item, entry, next, &map->elem[i], 
                        mapNode ) {
            hlist_del( &item->mapNode );
            mjItem_Delete( item );
        }
    }
    free( map );
    return true;
}
