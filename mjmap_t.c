#include <stdio.h>
#include "mjmap.h"

int main()
{
    mjmap map = mjMap_New(10241);

    mjstr str = mjstr_new();
    mjstr_copys(str, "value1");
    mjmap_add(map, "key1", str);
   
    mjstr_copys(str, "value2");
    mjmap_add(map, "key2", str);

    mjstr_copys(str, "value3");
    mjmap_add(map, "key3", str);

    mjstr_copys(str, "value4");
    mjmap_add(map, "key4", str );

    mjmap_del( map, "key4" );
    mjmap_del( map, "key2" );
    mjmap_del( map, "key1" );

    mjitem item = mjmap_GetNext( map, NULL );
    while ( item ) {
        printf( "key: %s, value: %s\n", item->key->str, item->value->str );
        item = mjmap_GetNext( map, item );
    }

    mjstr_delete(str);
    mjmap_delete(map);

    return 0;
}
