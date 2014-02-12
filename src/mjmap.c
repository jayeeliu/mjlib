#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mjlog.h"
#include "mjmap.h"

/*
===============================================================================
mjitem_new
  create new mjitem
===============================================================================
*/
static mjitem mjitem_new(const char* key, void* obj, mjProc obj_free,
    int type) {
  mjitem item = (mjitem) calloc(1, sizeof(struct mjitem));
  if (!item) {
    MJLOG_ERR("mjitem calloc error");
    return NULL;
  }
  item->key = mjstr_new(32);
  item->obj = obj;
  item->_obj_free = obj_free;
  item->_type = type;
  if (!item->key) {
    MJLOG_ERR("mjstr new error");
    free(item);
    return NULL;
  }
  mjstr_copys(item->key, key);
  INIT_LIST_HEAD(&item->_listnode);
  INIT_HLIST_NODE(&item->_mapnode);
  return item;
}

/*
===============================================================================
mjitem_delete
  delete mjitem
===============================================================================
*/
static inline bool mjitem_delete(mjitem item) {
  if (!item) return false;
  mjstr_delete(item->key);
  if (item->_type == MJITEM_STR) {
    mjstr_delete(item->obj);
  } else if (item->_type == MJITEM_OBJ && item->_obj_free) {
    item->_obj_free(item->obj);
  }
  free(item);
  return true;
}

/**
 * gen hash value, copy from redis
 */
static unsigned int genhashvalue(const void* key, int len)
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
mjmap_Search
  search item
===============================================================================
*/
mjitem mjmap_search(mjmap map, const char* key) {
  // get hash value and index
  unsigned int hashvalue = genhashvalue((void*)key, strlen(key));
  unsigned int index = hashvalue % map->_len;
  // search entry
  mjitem item = NULL;
  struct hlist_node *entry;
  hlist_for_each_entry(item, entry, &map->_elem[index], _mapnode) { 
    if (strcmp(item->key->data, key) == 0) return item;
  }
  return NULL;
}

/*
===============================================================================
mjmap_set
  set key and obj to mjmap
  return  -1 --- error
      -2 --- already exists
       0 --- success
===============================================================================
*/
int mjmap_set(mjmap map, const char* key, void* obj, mjProc obj_free, 
    int type) {
  unsigned int hashvalue = genhashvalue((void*)key, strlen(key));
  unsigned int index = hashvalue % map->_len;
  mjitem item = mjmap_search(map, key);
  if (item) return -2;
  item = mjitem_new(key, obj, obj_free, type);
  if (!item) {
    MJLOG_ERR("mjitem_new error");
    return -1;
  }
  // add to list and elem list
  list_add_tail(&item->_listnode, &map->_listhead);
  hlist_add_head(&item->_mapnode, &map->_elem[index]);
  return 0;
}

/*
===============================================================================
mjmap_Del
  delete one element from mjmap
===============================================================================
*/
bool mjmap_del(mjmap map, const char* key) {
  mjitem item = mjmap_search(map, key);
  if (!item) {
    MJLOG_ERR("mjmap_Del none");
    return false;
  }
  list_del(&item->_listnode);
  hlist_del(&item->_mapnode);
  mjitem_delete(item);
  return true;
}

/*
===============================================================================
mjmap_GetNext
  get next mjitem value 
===============================================================================
*/
mjitem mjmap_get_next(mjmap map, mjitem item) {
  // list is empty
  if (list_empty(&map->_listhead)) return NULL;
  if (item == NULL) {
    item = list_first_entry(&map->_listhead, struct mjitem, _listnode);
    return item;
  }
  // get next entry
  list_for_each_entry_continue(item, &map->_listhead, _listnode) break;
  if (&item->_listnode == &map->_listhead) return NULL;
  return item;
}

/*
===============================================================================
mjmap_New
  create new mjmap
===============================================================================
*/
mjmap mjmap_new(int mapsize) {
  mjmap map = (mjmap) calloc(1, sizeof(struct mjmap) + 
            mapsize * sizeof(struct hlist_node));
  if (!map) {
    MJLOG_ERR("mjmap calloc error");
    return NULL;
  }
  map->_len = mapsize;
  INIT_LIST_HEAD(&map->_listhead);
  for (int i = 0; i < mapsize; i++) INIT_HLIST_HEAD(&map->_elem[i]);
  return map;
}

/*
===============================================================================
mjmap_Delete
  delete mjmap
===============================================================================
*/
bool mjmap_delete(mjmap map) {
  // sanity check
  if (!map) return false;
  // get and clean mjitem
	mjitem item, tmp;
	list_for_each_entry_safe(item, tmp, &map->_listhead, _listnode) {
		list_del(&item->_listnode);
		mjitem_delete(item);
	}
  free(map);
  return true;
}
