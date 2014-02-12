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
  void*             obj;
  mjProc            _obj_free;
  int               _type;
  struct list_head  _listnode;
  struct hlist_node _mapnode;
};
typedef struct mjitem *mjitem;

struct mjmap {
  int               _len;                // hash length of elem
  struct list_head  _listhead;
  struct hlist_head _elem[0];            // element of mjitem
};
typedef struct mjmap *mjmap;


extern mjitem mjmap_get_next(mjmap map, mjitem item);
extern int    mjmap_set(mjmap map, const char* key, void* obj, mjProc obj_free, int type);
extern bool   mjmap_del(mjmap map, const char* key);
extern mjitem mjmap_search(mjmap map, const char* key);
extern mjmap  mjmap_new(int mapsize);
extern bool   mjmap_delete(mjmap map);


static inline mjstr mjmap_get_str(mjmap map, const char* key) {
  if (!map || !key) return NULL;
  mjitem item = mjmap_search(map, key);
  if (!item || item->_type != MJITEM_STR) return NULL;
  return item->obj;
}

static inline void* mjmap_get_obj(mjmap map, const char* key) {
  if (!map || !key) return NULL;
  mjitem item = mjmap_search(map, key);
  if (!item || item->_type != MJITEM_OBJ) return NULL;
  return item->obj;
}

static inline int mjmap_set_strs(mjmap map, const char* key, const char* str) {
  if (!map || !key) return -1;
  mjstr obj = mjstr_new(32);
  if (!obj) return -1; 
  mjstr_copys(obj, str);
  return mjmap_set(map, key, obj, NULL, MJITEM_STR);

}

static inline int mjmap_set_str(mjmap map, const char* key, mjstr value) {
  if (!value) return -1;
  return mjmap_set_strs(map, key, value->data);
}

static inline int mjmap_set_obj(mjmap map, const char* key, void* obj, 
    mjProc obj_free) {
  if (!map || !key) return -1;
  return mjmap_set(map, key, obj, obj_free, MJITEM_OBJ);
}

#endif
