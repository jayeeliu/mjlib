#include <stdio.h>
#include <stdlib.h>
#include "mjpool.h"

int main() {
  mjpool pool = mjpool_new(20);
  for (int i = 0; i < 100000000; ++i) {
    void *elem = mjpool_alloc(pool);
    if (!elem) {
      elem = malloc(100);
      printf("malloc\n");
    }
    if (!mjpool_free(pool, elem)) {
      free(elem);
    }
  }
  mjpool_delete(pool);
  return 0;
}
