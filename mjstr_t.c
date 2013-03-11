#include <stdio.h>
#include "mjstr.h"

int main()
{
    mjStr str = mjStr_New();
    mjStr_CopyS(str, "this is a test");
    mjStr_CatS(str, "test2");

    mjStrList strList = mjStrList_New();
    if ( !strList ) {
        printf( "mjStr_Split2 error" );
        return 1;
    }
    mjStr_Split( str, "te", strList );
    
    for (int i = 0; i < strList->length; i++) {
        printf("%s\n", strList->data[i]->str);
    }

    mjStr_Delete(str);
    mjStrList_Delete(strList);
    return 0;
}
