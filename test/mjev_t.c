#include <stdio.h>
#include <unistd.h>
#include "mjev.h"

void* PendingTest2( void* data ) {
    printf( "PendingTest2 Run!!!\n" );
    return NULL;
}

void* PendingTest1( void* data ) {
    printf( "PendingTest1 Run!!!\n" );
    mjev_add_pending( data, PendingTest1, data );
    mjev_add_pending( data, PendingTest2, data );
    return NULL;
}

int main() {
    mjev ev = mjev_New();
    if ( !ev ) {
        printf( "mjEV_New error" );
        return 1;
    }
    mjev_add_pending( ev, PendingTest1, ev );
    while ( 1 ) {
        mjev_Run( ev );
        sleep(1);
    }
    mjev_delete( ev );
    return 0;
}
