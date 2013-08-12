#include <stdio.h>
#include <stdlib.h>
#include "mjopt.h"

int main(int argc, char* argv[])
{
    mjopt_parse_conf("test.ini");
    int count;
    bool ret = mjopt_get_value_int(NULL, "c", &count);
    if (ret) {
      printf("%d\n", count);
    } else {
      printf("no value\n");
    }

    return 0;
}
