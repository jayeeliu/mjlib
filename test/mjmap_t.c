#include <stdio.h>
#include "mjmap.h"

int main()
{
    mjMap map = mjMap_New(10241);

    mjStr str = mjStr_New();
    mjStr_CopyS(str, "value1");
    mjMap_Add(map, "key1", str);
   
    mjStr_CopyS(str, "value2");
    mjMap_Add(map, "key2", str);

    mjStr_CopyS(str, "value3");
    mjMap_Add(map, "key3", str);

    mjStr_CopyS(str, "value4");
    mjMap_Add(map, "key4", str );
    mjMap_AddS(map, "key5", "value5");

//    mjMap_Del( map, "key4" );
//    mjMap_Del( map, "key2" );
//    mjMap_Del( map, "key1" );

    mjItem item = mjMap_GetNext( map, NULL );
    while ( item ) {
        printf( "key: %s, value: %s\n", item->key->data, item->value->data );
        item = mjMap_GetNext( map, item );
    }

    mjStr_Delete(str);
    mjMap_Delete(map);

    return 0;
}
