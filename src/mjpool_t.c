#include <stdio.h>
#include <stdlib.h>
#include "mjpool.h"

int main()
{
    mjpool pool = mjpool_new();


    for (int i = 0; i < 100000000; ++i) {
        void *e = mjpool_alloc(pool);
        if (!e) {
            e = malloc(100);
        }
        mjpool_free(pool, e);
    }
    
    mjpool_delete(pool);
    
    return 0;
}
