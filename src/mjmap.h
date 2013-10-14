#ifndef _MJMAP_H
#define _MJMAP_H

#include "mjlist.h"
#include "mjstr.h"
#include "mjproc.h"

#define MJITEM_STR  1
#define MJITEM_OBJ  2
#define MJITEM_MAP  3
#define MJITEM_LIST 4

struct mjitem {
  mjstr             key;
  mjstr             value_str;
  void*             value_obj;
  mjProc            value_obj_free;
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

extern mjstr  mjmap_get_str(mjmap map, const char* key);
extern int    mjmap_set_strs(mjmap map, const char* key, const char* value_str);

extern void*  mjmap_get_obj(mjmap map, const char* key);
extern int    mjmap_set_obj(mjmap map, const char* key, void* value_obj, mjProc value_obj_free);
extern bool   mjmap_del(mjmap map, const char* key);
extern mjitem mjmap_get_next(mjmap map, mjitem item);

extern mjmap  mjmap_new(int mapsize);
extern bool   mjmap_delete(mjmap map);

static inline int mjmap_set_str(mjmap map, const char* key, mjstr value) {
  if (!value) return -1;
  return mjmap_set_strs(map, key, value->data);
}

#endif
