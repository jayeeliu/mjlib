#include <stdio.h>
#include <unistd.h>
#include "mjev.h"

void* PendingTest2( void* data ) {
    printf( "PendingTest2 Run!!!\n" );
    return NULL;
}

void* PendingTest1( void* data ) {
    printf( "PendingTest1 Run!!!\n" );
    mjEV_AddPending( data, PendingTest1, data );
    mjEV_AddPending( data, PendingTest2, data );
    return NULL;
}

int main() {
    mjEV ev = mjEV_New();
    if ( !ev ) {
        printf( "mjEV_New error" );
        return 1;
    }
    mjEV_AddPending( ev, PendingTest1, ev );
    while ( 1 ) {
        mjEV_Run( ev );
        sleep(1);
    }
    mjEV_Delete( ev );
    return 0;
}
