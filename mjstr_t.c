#include <stdio.h>
#include "mjstr.h"

int main()
{
    mjstr str = mjstr_new();
    mjstr_copys(str, "this is a test");
    mjstr_cats(str, "test2");

    mjStrList strList = mjStrList_New();
    if ( !strList ) {
        printf( "mjstr_split2 error" );
        return 1;
    }
    mjstr_split( str, "te", strList );
    
    for (int i = 0; i < strList->length; i++) {
        printf("%s\n", strList->data[i]->str);
    }

    mjstr_delete(str);
    mjStrList_Delete(strList);
    return 0;
}
