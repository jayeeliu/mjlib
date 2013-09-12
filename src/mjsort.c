#include <stdlib.h>
#include "mjsort.h"
#include "mjlog.h"

struct mjsortitem {
  struct rb_node  node;
  long long       key;
  void*           value;
};
typedef struct mjsortitem* mjsortitem;

/*
===============================================================================
mjsortitem_New
  alloc new mjsortitem struct
===============================================================================
*/
static mjsortitem mjsortitem_new(long long key, void *value) {
  mjsortitem item = (mjsortitem) calloc(1, sizeof(struct mjsortitem));
  if (!item) return NULL;
  item->key   = key;
  item->value = value;
  return item;
}

/*
===============================================================================
mjsortitem_Delete
  delete mjsortitem
===============================================================================
*/
static bool mjsortitem_delete(mjsortitem item) {
  if (!item) return false;
  free(item);
  return true;
}

/*
===============================================================================
mjsort_SearchItem
  search key in mjsort return sortitem 
===============================================================================
*/
static mjsortitem mjsort_searchitem(mjsort sort, long long key) {
  struct rb_node* node = sort->tree_root.rb_node;
  while (node) {
    mjsortitem testItem = rb_entry(node, struct mjsortitem, node);
    long long testKey = testItem->key;
    if (testKey > key) {
      node = node->rb_left;
      continue;
    } else if (testKey < key) {
      node = node->rb_right;
      continue;
    }
    return testItem;
  }
  return NULL;
}

/*
===============================================================================
mjsort_Search
  search key in mjsort
===============================================================================
*/
void* mjsort_search(mjsort sort, long long key) {
  mjsortitem item = mjsort_searchitem(sort, key);
  if (item) return item->value;
  return NULL;
}

/*
===============================================================================
mjsort_Insert
  insert key and value into rbtree
===============================================================================
*/
bool mjsort_insert(mjsort sort, long long key, void *value) {
  // alloc new mjsortitem
  mjsortitem item = mjsortitem_new(key, value);
  if (!item) {
    MJLOG_ERR("mjsortitem_New error");
    return false;
  }
  // get position into newNode and parent
  struct rb_node** newNode = &sort->tree_root.rb_node;
  struct rb_node*  parent = NULL;
  while (*newNode) {
    mjsortitem testItem = rb_entry(*newNode, struct mjsortitem, node);
    long long testKey = testItem->key;
    parent = *newNode;
    if (testKey > item->key) {
      newNode = &(*newNode)->rb_left;
      continue;
    } else if (testKey < item->key) {
      newNode = &(*newNode)->rb_right;
      continue;
    }
    mjsortitem_delete(item);
    return false;
  }
  // insert into rbtree
  rb_link_node(&item->node, parent, newNode);
  rb_insert_color(&item->node, &sort->tree_root);
  return true;
}

/*
===============================================================================
mjsort_Erase
  erase one key
===============================================================================
*/
bool mjsort_erase(mjsort sort, long long key) {
  mjsortitem item = mjsort_searchitem(sort, key);
  if (!item) return false;
  rb_erase(&item->node, &sort->tree_root);
  mjsortitem_delete(item);
  return true;
}

/*
===============================================================================
mjsort_New
  create mjsort struct
===============================================================================
*/
mjsort mjsort_new() {
  mjsort sort = (mjsort) calloc(1, sizeof(struct mjsort));
  if (!sort) return NULL;
  sort->tree_root = RB_ROOT;
  return sort;
}

/*
===============================================================================
mjsort_Delete
  delete mjsort
===============================================================================
*/
bool mjsort_delete(mjsort sort) {
  if (!sort) return false;
  for (struct rb_node* node = rb_first(&sort->tree_root); node;
        node = rb_first(&sort->tree_root)) {
    mjsortitem item = rb_entry(node, struct mjsortitem, node);
    rb_erase(node, &sort->tree_root);
    mjsortitem_delete(item); 
  }
  free(sort);
  return true;
} 
