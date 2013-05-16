#include <stdio.h>
#include <stdlib.h>
#include "mjopt.h"

struct setting {
    int     value1;
    int     value2;
    int     value3;
    char    strval1[512];
    char    strval2[512];
    char    strval3[512];
};

int main(int argc, char* argv[])
{
    struct setting s;

    mjOpt_Define(NULL, "value1", MJOPT_INT, &s.value1, "10");
    mjOpt_Define(NULL, "value2", MJOPT_INT, &s.value2, "20");
    mjOpt_Define(NULL, "value3", MJOPT_INT, &s.value3, NULL);
    mjOpt_Define(NULL, "strval", MJOPT_STR, s.strval1, NULL);
    mjOpt_Define(NULL, "strval2", MJOPT_STR, s.strval2, NULL);
    mjOpt_Define(NULL, "strval3", MJOPT_STR, s.strval3, NULL);

    mjOpt_SetValue( NULL, "strval", "test3" );
    mjOpt_SetValue( NULL, "value1", "12345" );
    mjOpt_SetValue( NULL, "value3", "600" );
    mjOpt_SetValue( NULL, "strval2", "test1213" );

    mjOpt_ParseConf( "test.ini" );

    printf("%d\n", s.value1);
    printf("%d\n", s.value2);
    printf("%d\n", s.value3);
    printf("%s\n", s.strval1);
    printf("%s\n", s.strval2);
    printf("%s\n", s.strval3);
    return 0;
}
