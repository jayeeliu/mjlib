#include <stdlib.h>
#include "mjsort.h"
#include "mjlog.h"

/*
===============================================================================
mjSortItem_New
  alloc new mjsortitem struct
===============================================================================
*/
static mjSortItem mjSortItem_New(long long key, void *value) {
  mjSortItem item = (mjSortItem) calloc(1, sizeof(struct mjSortItem));
  if (!item) return NULL;
  item->key   = key;
  item->value = value;
  return item;
}

/*
===============================================================================
mjSort_Insert
  insert key and value into rbtree
===============================================================================
*/
bool mjSort_Insert(mjSort sort, long long key, void *value) {
  // alloc new mjSortItem
  mjSortItem item = mjSortItem_New(key, value);
  if (!item) {
    MJLOG_ERR("mjSortItem_New error");
    return false;
  }
  // get position into newNode and parent
  struct rb_node **newNode = &(sort->treeRoot.rb_node);
  struct rb_node *parent = NULL;
  while (*newNode) {
    mjSortItem testItem = container_of(*newNode, struct mjSortItem, node);
    long long testKey = testItem->key;
    parent = *newNode;
    if (testKey < item->key) {
      newNode = &((*newNode)->rb_left);
      continue;
    } else if (testKey > item->key) {
      newNode = &((*newNode)->rb_right);
      continue;
    }
    break; 
  }
  // insert into rbtree
  rb_link_node(&item->node, parent, newNode);
  rb_insert_color(&item->node, &sort->treeRoot);
  return true;
}

/*
===============================================================================
mjSort_New
  create mjSort struct
===============================================================================
*/
mjSort mjSort_New() {
  mjSort sort = (mjSort) calloc(1, sizeof(struct mjSort));
  if (!sort) return NULL;
  sort->treeRoot = RB_ROOT;
  return sort;
}

/*
===============================================================================
mjSort_Delete
  delete mjsort
===============================================================================
*/
bool mjSort_Delete(mjSort sort) {
  if (!sort) return false;
  free(sort);
  return true;
} 
