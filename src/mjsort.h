#ifndef _MJSORT_H
#define _MJSORT_H

#include <stdbool.h>
#include "mjrbtree.h"

struct mjSortItem {
  struct rb_node  node;
  long long       key;
  void            *value;
};
typedef struct mjSortItem *mjSortItem;

struct mjSort {
  struct rb_root treeRoot;
};
typedef struct mjSort *mjSort;

extern bool   mjSort_Insert(mjSort sort, long long key, void *value);
extern void*  mjSort_Search(mjSort sort, long long key);
extern mjSort mjSort_New();
extern bool   mjSort_Delete(mjSort sort);

#endif
