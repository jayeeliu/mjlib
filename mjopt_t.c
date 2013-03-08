#include <stdio.h>
#include <stdlib.h>
#include "mjopt.h"

struct setting {
    int value1;
    int value2;
    int value3;
    char strval1[512];
    char strval2[512];
};

int main(int argc, char* argv[])
{
    struct setting s;

    mjOpt_Define(NULL, "value1", MJOPT_INT, &s.value1, "10", "v1", 1, "set value1");
    mjOpt_Define(NULL, "value2", MJOPT_INT, &s.value2, "20", "v2", 1, "set value2");
    mjOpt_Define(NULL, "value3", MJOPT_INT, &s.value3, NULL, "v3", 1, "set value3");
    mjOpt_Define(NULL, "strval", MJOPT_STR, s.strval1, NULL, "s1", 1, "set strvalue1");
    mjOpt_Define(NULL, "strval2", MJOPT_STR, s.strval2, NULL, "s2", 1, "set strvalue2");

    mjOpt_SetValue( NULL, "strval", "test3" );
    mjOpt_SetValue( NULL, "value1", "a101a" );
    printf("%d\n", s.value1);
    printf("%d\n", s.value2);
    printf("%d\n", s.value3);
    printf("%s\n", s.strval1);
    printf("%s\n", s.strval2);

    mjOpt_ParseCmd( argc, argv );
    mjOpt_HelpString();

    return 0;
}
