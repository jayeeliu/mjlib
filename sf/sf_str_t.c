#include <stdio.h>
#include "sf_str.h"

int main() {
  sf_str str = sf_str_new(100);
  sf_str_copys(str, "       ");
  sf_slist slist = sf_slist_new();
  sf_str_split(str, " ", slist);
  for (int i=0; i<slist->len; i++) {
    printf("%d\n", slist->data[i]->len);
  }
  sf_slist_del(slist);
  sf_str_del(str);
  return 0;
}
