#include <stdio.h>
#include <unistd.h>
#include "mjthread.h"

void* Routine( void* arg )
{
    char* name = arg;

    printf("Run OK: %s\n", name);
    return NULL;
}

void* PreRoutine( void* arg )
{
    printf("Before thread\n");
    return NULL;
}

void* PostRoutine( void* arg )
{
    printf("After thread\n");
    return NULL;
}

int main()
{
    mjThread thread = mjThread_New();
    int count = 0;
   
    mjThread_SetPrePost( thread, PreRoutine, PostRoutine ); 
    while ( count < 10 ) {
        mjThread_AddWork( thread, Routine, "1" );
        count++;
    }
    sleep(10);
    mjThread_Delete( thread );
    return 0;
}
