#include <stdio.h>
#include <string.h>
#include "mjsql.h"

int main()
{
    mjsql handle = mjsql_new("127.0.0.1", "root", "", "test", 3306);

    char *str = "select * from tt";
    mjsql_query(handle, str, strlen(str));

    MYSQL_RES* result = mjsql_store_result(handle);

    unsigned int num_fields;
    MYSQL_ROW row;
    while ((row = mjsql_fetch_row(result, &num_fields))) {
        for (int i = 0; i < num_fields; i++) {
            printf("%s ", row[i]);
        }
        printf("\n");

    }
    mjsql_free_result(result);
    
    mjsql_delete(handle);

    return 0;
}
