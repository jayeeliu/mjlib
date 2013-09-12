#include "mjsort.h"

int main() {
  mjsort sort = mjsort_new();
  mjsort_insert(sort, 10, "value 10");
  mjsort_insert(sort, 20, "value 20");
  mjsort_insert(sort, 5, "value 5");
  mjsort_insert(sort, 30, "value 30");
  mjsort_insert(sort, 10, "value 10--2");
  mjsort_erase(sort, 20);
  mjsort_erase(sort, 30);
/*
  for (struct rb_node* node = rb_first(&sort->tree_root); node;
    node = rb_next(node)) {
    printf("key:%lld, value:%s\n", 
        rb_entry(node, struct mjsortItem, node)->key,
        (char*)rb_entry(node, struct mjsortItem, node)->value);
  }
  */
  char* ret = mjsort_search(sort, 10);
  printf("found: %s\n", ret);

  mjsort_delete(sort);
  return 0;
}
