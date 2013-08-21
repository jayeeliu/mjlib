#include <stdio.h>
#include <string.h>
#include "mjsql.h"

int main() {
	mjsql handle = mjsql_new("127.0.0.1", "root", "", "test", 3306);
	if (!handle) {
		printf("mjsql handle alloc error\n");
		return 1;
	}
	mjsql_delete(handle);

	return 0;
}
