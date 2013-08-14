#ifndef _MJMAP_H
#define _MJMAP_H

#include "mjlist.h"
#include "mjstr.h"

struct mjitem {
  mjstr             key;
  mjstr             value_str;
  void*             value_obj;
  int               type;
  struct list_head  listNode;
  struct hlist_node mapNode;
};
typedef struct mjitem *mjitem;

struct mjmap {
  int               len;                // hash length of elem
  struct list_head  listHead;
  struct hlist_head elem[0];            // element of mjitem
};
typedef struct mjmap *mjmap;

extern int    mjmap_add(mjmap map, const char* key, mjstr value);
extern int    mjmap_adds(mjmap map, const char* key, const char* value);
extern int    mjmap_del(mjmap map, const char* key);
extern mjstr  mjmap_get(mjmap map, const char* key);
extern mjitem mjmap_get_next(mjmap map, mjitem item);

extern mjmap  mjmap_new(int mapsize);
extern bool   mjmap_delete(mjmap map);

#endif
