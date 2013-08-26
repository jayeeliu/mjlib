#include "mjstr2.h"
#include <stdio.h>

void t1() {
  mjstr str = mjstr_new(0);
  mjstr_copys(str, "t");
  printf("%s\n", mjstr_tochar(str));
  mjstr_cats(str, "123412312");
  printf("%s\n", mjstr_tochar(str));
  mjstr_consume(str, 5);
  printf("%d\n", mjstr_search(str, "3"));
  mjstr_delete(str);
}

void t2() {
  mjstr str = mjstr_new(1024);
  mjstr_copys(str, " this is a test");
  mjstr_strim(str);
  printf("%s\n", mjstr_tochar(str));
  printf("%d\n", mjstr_get_length(str));
  mjstr_delete(str);
}

void t3() {
  mjstr str = mjstr_new(80);
  mjstr_copys(str, "this is a test string 10 20 30");
  mjstr_consume(str, 3);
  mjstrlist str_list = mjstrlist_new();
  mjstr_split(str, " ", str_list);
  mjstr inner;
  int i = 0;
  while (1) {
    inner = mjstrlist_get(str_list, i);
    if (!inner) break;
    printf("%s\n", mjstr_tochar(inner));
    i++;
  }
  mjstrlist_delete(str_list);
  mjstr_delete(str);
}

int main() {
  t1();
  t2();
  t3();
  return 0;
}
