#include <stdio.h>
#include "mjmap.h"

int main()
{
    mjmap map = mjmap_new(10241);

    mjstr str = mjstr_new();
    mjstr_copys(str, "value1");
    mjmap_set_str(map, "key1", str);
   
    mjstr_copys(str, "value2");
    mjmap_set_str(map, "key2", str);

    mjstr_copys(str, "value3");
    mjmap_set_str(map, "key3", str);

    mjstr_copys(str, "value4");
    mjmap_set_str(map, "key4", str);
    mjmap_set_strs(map, "key5", "value5");

    mjmap_set_obj(map, "obj1", "obj1 is here");
    mjmap_del(map, "key3");

    mjitem item = mjmap_get_next(map, NULL);
    while (item) {
        if (item->type == MJITEM_STR) {
          printf("key: %s, str value: %s\n", item->key->data, item->value_str->data);
        } else {
          printf("key: %s, obj value: %s\n", item->key->data, (char*)item->value_obj);
        }
        item = mjmap_get_next(map, item);
    }

    mjstr_delete(str);
    mjmap_delete(map);

    return 0;
}
