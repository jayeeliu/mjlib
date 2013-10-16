#include "mjstr.h"
#include <stdio.h>

void t1() {
  mjstr str = mjstr_new(0);
  mjstr_copys(str, "t");
  printf("%s\n", str->data);
  mjstr_cats(str, "123412312");
  printf("%s\n", str->data);
  mjstr_consume(str, 5);
  printf("%d\n", mjstr_search(str, "3"));
  mjstr_delete(str);
}

void t2() {
  mjstr str = mjstr_new(1024);
  mjstr_copys(str, " this is a test");
  mjstr_strim(str);
  printf("%s\n", str->data);
  printf("%d\n", str->len);
  mjstr_delete(str);
}

void t3() {
  mjstr str = mjstr_new(80);
  mjstr_copys(str, "this is a test string 10 20 30");
  mjstr_consume(str, 3);
  mjslist slist = mjslist_new();
  mjstr_split(str, " ", slist);
  mjstr inner;
  int i = 0;
  while (1) {
    inner = mjslist_get(slist, i);
    if (!inner) break;
    printf("%s\n", inner->data);
    i++;
  }
  mjslist_delete(slist);
  mjstr_delete(str);
}

void t4() {
  mjstr str = mjstr_new(20);
  printf("t4: %s\n", str->data);
  mjslist slist = mjslist_new();
  mjstr_split(str, " ", slist);
  printf("t4 slist: %d\n", slist->len);
  printf("t4 data: %d\n", slist->data[0]->len);
  mjstr_delete(str);
  mjslist_delete(slist);
}

int main() {
  t1();
  t2();
  t3();
  t4();
  return 0;
}
