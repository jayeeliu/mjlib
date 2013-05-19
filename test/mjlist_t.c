#include <stdio.h>
#include <stdlib.h>
#include "mjlist.h"

struct tlist {
    struct list_head list;
    int value;
};

struct list_head tlist_head;

struct tlist* GetNext( struct tlist* pos, struct list_head* tlist_head ) {
    // list is emptyy
    if ( list_empty( tlist_head ) ) return NULL;
    // get first entry
    if ( pos == NULL ) {
        pos = list_first_entry( tlist_head, struct tlist, list ); 
        return pos;
    }
    // get next entry
    list_for_each_entry_continue( pos, tlist_head, list ) break;
    if ( &pos->list == tlist_head ) return NULL;
    return pos;
}

int main()
{
    INIT_LIST_HEAD(&tlist_head);

    for (int i = 0; i < 10; i++) {
        struct tlist *nlist = (struct tlist*) calloc (1, sizeof(struct tlist));
        if (!nlist) break;
        nlist->value = i;
        list_add_tail(&nlist->list, &tlist_head);
    }
   
    struct tlist* pos;
    pos = NULL;
    pos = GetNext( pos, &tlist_head );
    while ( pos ) {
        printf( "%d\n", pos->value );
        pos = GetNext( pos, &tlist_head );
    }

    return 0;
}
