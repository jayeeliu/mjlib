#include <stdio.h>
#include <unistd.h>
#include "mjthread.h"

void* Routine( void* arg )
{
    static int count = 0;
    printf("count: %4d\n", count++);
    sleep(1);
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
    mjThread thread = mjThread_NewLoop( Routine, NULL );
  
    sleep( 5 ); 
    mjThread_Delete( thread );

    return 0;
}
