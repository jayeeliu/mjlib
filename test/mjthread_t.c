#include <stdio.h>
#include <unistd.h>
#include "mjthread.h"

void* MyWorker( void* arg )
{
    printf( "Worker Run\n" );
    return NULL;
}

int main()
{
    mjThread thread = mjThread_New();
    int value = 0;
    while ( 1 ) {
        mjThread_AddWork( thread, MyWorker,  &value );
        value++;
        sleep( 1 );
    }
    mjThread_Delete( thread );

    return 0;
}
