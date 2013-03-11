#include <stdio.h>
#include "mjstr.h"

int main()
{
    mjStr str = mjStr_New();
    mjStr_CopyS(str, "this is a test");
    mjStr_CatS(str, "test2");

    int ret = mjStr_Search( str, "is" );
    printf( "found in %d\n", ret );

    mjStr_Capitablize( str );
    printf("%s\n", str->data );

    mjStrList strList = mjStrList_New();
    if ( !strList ) {
        printf( "mjStr_Split2 error" );
        return 1;
    }
    mjStr_Split( str, "TE", strList );
    
    for (int i = 0; i < strList->length; i++) {
        printf("%s\n", strList->data[i]->data);
    }

    mjStr_Delete(str);
    mjStrList_Delete(strList);
    return 0;
}
