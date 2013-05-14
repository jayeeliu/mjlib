#include <stdio.h>
#include "mjev2.h"

void* PendingTest( void* data ) {
    printf( "PendingTest Run!!!\n" );
    mjEV2_Pending( data, PendingTest, data );
    return NULL;
}

int main() {
    mjEV2 ev = mjEV2_New();
    if ( !ev ) {
        printf( "mjEV2_New error" );
        return 1;
    }
    mjEV2_Pending( ev, PendingTest, ev );
    while ( 1 ) {
        mjEV2_Run( ev );
        sleep(1);
    }
    mjEV2_Delete( ev );
    return 0;
}
