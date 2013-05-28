#include "mjsort.h"

int main() {
  mjSort sort = mjSort_New();
  mjSort_Insert(sort, 10, "value 10");
  mjSort_Insert(sort, 20, "value 20");
  mjSort_Insert(sort, 5, "value 5");
  mjSort_Insert(sort, 30, "value 30");
  mjSort_Insert(sort, 10, "value 10--2");
  mjSort_Erase(sort, 20);
  mjSort_Erase(sort, 30);
  mjSort_Erase(sort, 10);

  for (struct rb_node* node = rb_first(&sort->treeRoot); node;
        node = rb_next(node)) {
    printf("key:%lld, value:%s\n", 
        rb_entry(node, struct mjSortItem, node)->key,
        (char*)rb_entry(node, struct mjSortItem, node)->value);
  }
  char* ret = mjSort_Search(sort, 10);
  printf("found: %s\n", ret);

  mjSort_Delete(sort);
  return 0;
}
