#ifndef _MJMAP_H
#define _MJMAP_H

#include "mjlist.h"
#include "mjstr.h"

struct mjitem {
    mjStr               key;
    mjStr               value;
    struct list_head    listNode;
    struct hlist_node   mapNode;
};
typedef struct mjitem* mjitem;

struct mjmap {
    int                 len;                            /* hash length of elem */
    struct list_head    listHead;
    struct hlist_head   elem[0];                        /* element of mjitem */
};
typedef struct mjmap* mjmap;

extern int      mjMap_Add( mjmap map, const char* key, mjStr value );
extern int      mjMap_Del( mjmap map, const char* key );
extern mjStr    mjMap_Get( mjmap map, const char* key );
extern mjitem   mjmap_GetNext( mjmap map, mjitem item );

extern mjmap    mjMap_New( int mapsize );
extern bool     mjMap_Delete( mjmap map );

#endif
