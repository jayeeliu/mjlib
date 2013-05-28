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
mjSortItem_Delete
  delete mjsortitem
===============================================================================
*/
static bool mjSortItem_Delete(mjSortItem item) {
  if (!item) return false;
  free(item);
  return true;
}

/*
===============================================================================
mjSort_SearchItem
  search key in mjsort return sortitem 
===============================================================================
*/
static mjSortItem mjSort_SearchItem(mjSort sort, long long key) {
  struct rb_node* node = sort->treeRoot.rb_node;
  while (node) {
    mjSortItem testItem = rb_entry(node, struct mjSortItem, node);
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
mjSort_Search
  search key in mjsort
===============================================================================
*/
void* mjSort_Search(mjSort sort, long long key) {
  mjSortItem item = mjSort_SearchItem(sort, key);
  if (item) return item->value;
  return NULL;
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
  struct rb_node **newNode = &sort->treeRoot.rb_node;
  struct rb_node *parent = NULL;
  while (*newNode) {
    mjSortItem testItem = rb_entry(*newNode, struct mjSortItem, node);
    long long testKey = testItem->key;
    parent = *newNode;
    if (testKey > item->key) {
      newNode = &((*newNode)->rb_left);
      continue;
    } else if (testKey < item->key) {
      newNode = &((*newNode)->rb_right);
      continue;
    }
    mjSortItem_Delete(item);
    return false;
  }
  // insert into rbtree
  rb_link_node(&item->node, parent, newNode);
  rb_insert_color(&item->node, &sort->treeRoot);
  return true;
}

/*
===============================================================================
mjSort_Erase
  erase one key
===============================================================================
*/
bool mjSort_Erase(mjSort sort, long long key) {
  mjSortItem item = mjSort_SearchItem(sort, key);
  if (!item) return false;
  rb_erase(&item->node, &sort->treeRoot);
  mjSortItem_Delete(item);
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
  for (struct rb_node* node = rb_first(&sort->treeRoot); node;
        node = rb_first(&sort->treeRoot)) {
    mjSortItem item = rb_entry(node, struct mjSortItem, node);
    rb_erase(node, &sort->treeRoot);
    mjSortItem_Delete(item); 
  }
  free(sort);
  return true;
} 
