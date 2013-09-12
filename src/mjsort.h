#ifndef _MJSORT_H
#define _MJSORT_H

#include <stdbool.h>
#include "mjrbtree.h"

struct mjsort {
  struct rb_root tree_root;
};
typedef struct mjsort *mjsort;

extern bool   mjsort_erase(mjsort sort, long long key);
extern bool   mjsort_insert(mjsort sort, long long key, void *value);
extern void*  mjsort_search(mjsort sort, long long key);
extern mjsort mjsort_new();
extern bool   mjsort_delete(mjsort sort);

#endif
