#include <stdio.h>
#include <unistd.h>
#include "mjthreadpool2.h"

void* Routine( void* arg )
{
    long a = 1;
    for( int i=1; i<1000; i++) {
        a = a * i;
    }
    return NULL;
}
 
int main()
{
    int count = 0;
    mjThreadPool2 tPool = mjThreadPool2_New( 10 );

    for(int i=0; i<100; i++) {
        bool ret = mjThreadPool2_AddWork( tPool, Routine, NULL );
        if ( !ret ) {
            mjThread_RunOnce( Routine, NULL );
            count++;
        }
    }

    sleep(1);
    mjThreadPool2_Delete( tPool );

    printf("%d\n", count);
    return 0;
}
