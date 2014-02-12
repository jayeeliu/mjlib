#include <stdio.h>
#include <string.h>
#include "mjsql.h"

int main() {
  mjsql handle = mjsql_new("127.0.0.1", "root", "", "test", 3306);
  if (!handle) {
    printf("mjsql handle alloc error\n");
    return 1;
  }
  mjsql_set_autocommit(handle, false);
  char* sql_str = "begin";
  int retval = mjsql_query(handle, sql_str, strlen(sql_str)); 
  if (retval) {
    printf("mjsql_query error\n");
    mjsql_delete(handle);
    return 1;
  }
  char* sql_str2 = "insert into t1 values(80, \"test8\")";
  retval = mjsql_query(handle, sql_str2, strlen(sql_str2));
  if (retval) {
    printf("mjsql_query error\n");
    mjsql_delete(handle);
    return 1;
  }

  char* sql_str3 = "rollback";
  retval = mjsql_query(handle, sql_str3, strlen(sql_str3));
  if (retval) {
    printf("mjsql_query error\n");
    mjsql_delete(handle);
    return 1;
  }

  mjsql_store_result(handle);
  printf("rows: %d\n", mjsql_get_rows_num(handle));
  while (mjsql_next_row(handle)) {
    printf("%s\n", mjsql_fetch_row_field(handle, 0));
  }
  mjsql_delete(handle);

  return 0;
}
