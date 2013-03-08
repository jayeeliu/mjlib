#ifndef __MJMAP_H
#define __MJMAP_H

#include "mjlist.h"
#include "mjstr.h"

struct mjitem {
    mjstr               key;
    mjstr               value;
    struct hlist_node   map_node;
    struct mjitem*      prev;
    struct mjitem*      next;
};
typedef struct mjitem* mjitem;

struct mjmap {
    int                 len;                            /* hash length of elem */
    int                 itemcount;                      /* item count in this map */
    mjitem              head;
    mjitem              tail;
    struct hlist_head   elem[0];                        /* element of mjitem */
};
typedef struct mjmap* mjmap;

extern int      mjmap_add( mjmap map, const char* key, mjstr value );
extern int      mjmap_del( mjmap map, const char* key );
extern mjstr    mjmap_get( mjmap map, const char* key );
extern mjitem   mjmap_GetNext( mjmap map, mjitem item );

extern mjmap    mjmap_new( int mapsize );
extern void     mjmap_delete( mjmap map );

#endif
