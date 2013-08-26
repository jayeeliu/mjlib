#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mjlog.h"
#include "mjmap.h"

/*
===============================================================================
mjitem_New
  create new mjitem struct
===============================================================================
*/
static mjitem mjitem_new_strs(const char* key, const char* value_str) {
  // alloc mjitem
  mjitem item = (mjitem) calloc(1, sizeof(struct mjitem));
  if (!item) {
    MJLOG_ERR("mjitem calloc error");
    return NULL;
  }
  // set key and value 
  item->key   = mjstr_new(32);
  item->value_str = mjstr_new(32);
  item->value_obj = NULL;
  item->type = MJITEM_STR;
  if (!item->key || !item->value_str) {
    MJLOG_ERR("mjstr new error");
    mjstr_delete(item->key);
    mjstr_delete(item->value_str);
    free(item);
    return NULL;
  }
  // set key and value
  mjstr_copys(item->key, key);
  mjstr_copys(item->value_str, value_str);
  // init list and map list
  INIT_LIST_HEAD(&item->listNode);
  INIT_HLIST_NODE(&item->mapNode);
  return item;
}

/*
===============================================================================
mjitem_new_obj
  alloc new mjitem store obj
===============================================================================
*/
static mjitem mjitem_new_obj(const char* key, void* value_obj,
    mjProc value_obj_free) {
  mjitem item = (mjitem) calloc(1, sizeof(struct mjitem));
  if (!item) {
    MJLOG_ERR("mjitem calloc error");
    return NULL;
  }
  item->key = mjstr_new(32);
  item->value_str = NULL;
  item->value_obj = value_obj;
  item->value_obj_free = value_obj_free;
  item->type = MJITEM_OBJ;
  if (!item->key) {
    MJLOG_ERR("mjstr new error");
    free(item);
    return NULL;
  }
  mjstr_copys(item->key, key);
  // init list and maplist
  INIT_LIST_HEAD(&item->listNode);
  INIT_HLIST_NODE(&item->mapNode);
  return item;
}

/*
===============================================================================
mjitem_Delete
  delete mjitem
===============================================================================
*/
static bool mjitem_delete(mjitem item) {
  // sanity check
  if (!item) {
    MJLOG_ERR("item is null");
    return false;
  }
  // free key
  mjstr_delete(item->key);
  if (item->type == MJITEM_STR) {
    mjstr_delete(item->value_str);
  } else if (item->type == MJITEM_OBJ && item->value_obj_free) {
    item->value_obj_free(item->value_obj);
  }
  // free struct
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
static mjitem mjmap_search(mjmap map, const char* key) {
  // get hash value and index
  unsigned int hashvalue = genhashvalue((void*)key, strlen(key));
  unsigned int index = hashvalue % map->len;
  // search entry
  mjitem item = NULL;
  struct hlist_node *entry;
  hlist_for_each_entry(item, entry, &map->elem[index], mapNode) { 
    if (strcmp(mjstr_tochar(item->key), key) == 0) return item;
  }
  return NULL;
}

/*
===============================================================================
mjmap_Add
  add key and value to mjmap, call mjmap_AddS
  return  -1 --- error
      -2 --- already exists
       0 --- success
===============================================================================
*/
int mjmap_set_str(mjmap map, const char* key, mjstr value) {
  return mjmap_set_strs(map, key, mjstr_tochar(value));
}

/*
===============================================================================
mjmap_AddS
  add key and value to mjmap, call mjmap_AddS
  return  -1 --- error
      -2 --- already exists
       0 --- success
===============================================================================
*/
int mjmap_set_strs(mjmap map, const char* key, const char* value_str) {
  // get hash value and index
  unsigned int hashvalue = genhashvalue((void*)key, strlen(key));
  unsigned int index = hashvalue % map->len;
  // search entry
  mjitem item = mjmap_search(map, key);
  if (item) return -2;
  // generator a new mjitem
  item = mjitem_new_strs(key, value_str);
  if (!item) {
    MJLOG_ERR("mjitem_New error");
    return -1;
  }
  // add to list and elem list
  list_add_tail(&item->listNode, &map->listHead);
  hlist_add_head(&item->mapNode, &map->elem[index]);
  return 0;
}

/*
===============================================================================
mjmap_set_obj
  set mjmap object
===============================================================================
*/
int mjmap_set_obj(mjmap map, const char* key, void* value_obj, 
    mjProc value_obj_free) {
  unsigned int hashvalue = genhashvalue((void*)key, strlen(key));
  unsigned int index = hashvalue % map->len;
  mjitem item = mjmap_search(map, key);
  if (item) return -2;
  item = mjitem_new_obj(key, value_obj, value_obj_free);
  if (!item) {
    MJLOG_ERR("mjitem_new error");
    return -1;
  }
  // add to list and elem list
  list_add_tail(&item->listNode, &map->listHead);
  hlist_add_head(&item->mapNode, &map->elem[index]);
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
  list_del(&item->listNode);
  hlist_del(&item->mapNode);
  mjitem_delete(item);
  return true;
}

/*
===============================================================================
mjmap_Get
  get mjstr from key
===============================================================================
*/
mjstr mjmap_get_str(mjmap map, const char* key) {
  // sanity check
  if (!map || !key) return NULL;
  // search mjitem 
  mjitem item = mjmap_search(map, key);
  if (!item || item->type != MJITEM_STR) return NULL;
  return item->value_str;
}

void* mjmap_get_obj(mjmap map, const char* key) {
  if (!map || !key) return NULL;
  // search mjitem
  mjitem item = mjmap_search(map, key);
  if (!item || item->type != MJITEM_OBJ) return NULL;
  return item->value_obj;
}
/*
===============================================================================
mjmap_GetNext
  get next mjitem value 
===============================================================================
*/
mjitem mjmap_get_next(mjmap map, mjitem item) {
  // list is empty
  if (list_empty(&map->listHead)) return NULL;
  if (item == NULL) {
    item = list_first_entry(&map->listHead, struct mjitem, listNode);
    return item;
  }
  // get next entry
  list_for_each_entry_continue(item, &map->listHead, listNode) break;
  if (&item->listNode == &map->listHead) return NULL;
  return item;
}

/*
===============================================================================
mjmap_New
  create new mjmap
===============================================================================
*/
mjmap mjmap_new(int mapsize) {
  // create map struct
  mjmap map = (mjmap) calloc(1, sizeof(struct mjmap) + 
            mapsize * sizeof(struct hlist_node));
  if (!map) {
    MJLOG_ERR("mjmap calloc error");
    return NULL;
  }
  // set mjmap fields
  map->len = mapsize;
  // init list
  INIT_LIST_HEAD(&map->listHead);
  // init hash list
  for (int i = 0; i < mapsize; i++) INIT_HLIST_HEAD(&map->elem[i]);
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
	/*
  for (int i = 0; i < map->len; i++) {
    mjitem item = NULL;
    struct hlist_node *entry, *next;
    hlist_for_each_entry_safe(item, entry, next, &map->elem[i], mapNode) {
      hlist_del(&item->mapNode);
      mjitem_delete(item);
    }
  }
	*/
	mjitem item, tmp;
	list_for_each_entry_safe(item, tmp, &map->listHead, listNode) {
		list_del(&item->listNode);
		mjitem_delete(item);
	}
  free(map);
  return true;
}
