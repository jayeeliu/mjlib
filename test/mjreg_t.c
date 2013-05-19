#include <stdio.h>
#include "mjreg.h"

int main()
{
    mjReg reg = mjReg_New("^(\\w\\w\\w)-([0-9][0-9][0-9])");
    if (!reg) {
        printf("reg create error\n");
        return 1;
    }

    mjStrList result = mjStrList_New();
    
    mjReg_Search(reg, "abc-123, def-234", result);
    for (int i = 0; i < 10; i++) {
        mjStr str = mjStrList_Get( result, i );
        if ( str ) {
            printf("%d, %s\n", i, str->data);
        }
    }

    mjStrList_Delete( result );
    mjReg_Delete(reg);

    return 0;
}
