#ifndef _MJMAP_H
#define _MJMAP_H

#include "mjlist.h"
#include "mjstr.h"

struct mjItem {
    mjStr               key;
    mjStr               value;
    struct list_head    listNode;
    struct hlist_node   mapNode;
};
typedef struct mjItem* mjItem;

struct mjMap {
    int                 len;                            /* hash length of elem */
    struct list_head    listHead;
    struct hlist_head   elem[0];                        /* element of mjItem */
};
typedef struct mjMap* mjMap;

extern int      mjMap_Add( mjMap map, const char* key, mjStr value );
extern int      mjMap_Del( mjMap map, const char* key );
extern mjStr    mjMap_Get( mjMap map, const char* key );
extern mjItem   mjMap_GetNext( mjMap map, mjItem item );

extern mjMap    mjMap_New( int mapsize );
extern bool     mjMap_Delete( mjMap map );

#endif
