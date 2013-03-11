#include <stdio.h>
#include <stdlib.h>
#include "mjpool.h"

int main()
{
    mjPool pool = mjPool_New();


    for (int i = 0; i < 10000000; ++i) {
        void *elem = mjPool_Alloc( pool );
        if ( !elem ) {
            elem = malloc( 100 );
        }
        
        if ( !mjPool_Free( pool, elem ) ) {
            free( elem );
        }
    }
    
    mjPool_Delete( pool );
    
    return 0;
}
