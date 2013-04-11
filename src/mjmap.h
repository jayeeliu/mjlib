#ifndef __MJMAP_H
#define __MJMAP_H

#include "mjlist.h"
#include "mjstr.h"

struct mjitem {
    mjStr               key;
    mjStr               value;
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

extern int      mjMap_Add( mjmap map, const char* key, mjStr value );
extern int      mjMap_Del( mjmap map, const char* key );
extern mjStr    mjMap_Get( mjmap map, const char* key );
extern mjitem   mjmap_GetNext( mjmap map, mjitem item );

extern mjmap    mjMap_New( int mapsize );
extern void     mjMap_Delete( mjmap map );

#endif
