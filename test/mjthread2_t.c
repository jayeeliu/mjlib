#include <stdio.h>
#include "mjthread.h"

void* Routine( void* arg )
{
    char* name = arg;

    printf("Run OK: %s\n", name);
    return NULL;
}

int main()
{
    mjThread thread1 = mjThread_New();
    mjThread thread2 = mjThread_New();
    int count = 0;
    
    while ( count < 10000 ) {
        bool ret = mjThread_AddWork( thread1, Routine, "1" );
        if ( !ret ) {

            ret = mjThread_AddWork( thread2, Routine, "2" );
            if ( !ret ) {
                printf("ADD ERROR\n");
            }
        }
        count++;
    }
    mjThread_Delete( thread1 );
    mjThread_Delete( thread2 );
    return 0;
}
