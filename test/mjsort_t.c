#include "mjsort.h"

int main() {
  mjsort sort = mjsort_new();
  mjsort_insert(sort, 10, "value 10");
  mjsort_insert(sort, 20, "value 20");
  mjsort_insert(sort, 5, "value 5");
  mjsort_insert(sort, 30, "value 30");
  mjsort_insert(sort, 10, "value 10--2");

  mjsortitem next = mjsort_next(sort, NULL);
  while (next) {
    printf("%lld, %s\n", next->key, (char*)next->value);
    next = mjsort_next(sort, next);
  }

  mjsort_delete(sort);
  return 0;
}
