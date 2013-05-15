#include <stdio.h>
#include <unistd.h>
#include "mjev2.h"

void* PendingTest2( void* data ) {
    printf( "PendingTest2 Run!!!\n" );
    return NULL;
}

void* PendingTest1( void* data ) {
    printf( "PendingTest1 Run!!!\n" );
    mjEV2_AddPending( data, PendingTest1, data );
    mjEV2_AddPending( data, PendingTest2, data );
    return NULL;
}

int main() {
    mjEV2 ev = mjEV2_New();
    if ( !ev ) {
        printf( "mjEV2_New error" );
        return 1;
    }
    mjEV2_AddPending( ev, PendingTest1, ev );
    while ( 1 ) {
        mjEV2_Run( ev );
        sleep(1);
    }
    mjEV2_Delete( ev );
    return 0;
}
