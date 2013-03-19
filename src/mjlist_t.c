#include <stdio.h>
#include <stdlib.h>
#include "mjlist.h"

struct tlist {
    struct list_head list;
    int value;
};

struct list_head tlist_head;

int main()
{
    INIT_LIST_HEAD(&tlist_head);

    for (int i = 0; i < 10; i++) {
        struct tlist *nlist = (struct tlist*) calloc (1, sizeof(struct tlist));
        if (!nlist) break;

        nlist->value = i;
        list_add_tail(&nlist->list, &tlist_head);
    }
/*
    struct tlist *pos;
    struct tlist *tmp;
    list_for_each_entry_safe(pos, tmp, &tlist_head, list) {
        printf("%d\n", pos->value);
        list_del(&pos->list);
        free(pos);
    }

    if (list_empty(&tlist_head)) {
        printf("tlist is empty\n");
    }
*/
/*
    struct tlist *pos;
    while (!list_empty(&tlist_head)) {
        pos = list_first_entry(&tlist_head, struct tlist, list);
        printf("%d\n", pos->value);
        list_del(&pos->list);
    }
*/
    struct tlist *pos;
    list_for_each_entry(pos, &tlist_head, list) {
        printf("value: %d\n", pos->value);
    }
    printf("out of loop\n");
    
    return 0;
}
