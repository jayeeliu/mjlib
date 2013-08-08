#include <stdio.h>
#include <unistd.h>
#include "mjthreadpool.h"

void* Routine(void* arg) {
    long a = 1;
    for(int i = 1; i < 2; i++) {
        a = a * i;
    }
    printf("%ld\n", a);
    return NULL;
}
 
int main()
{
    mjThreadPool tPool = mjThreadPool_New(10);

    for(int i = 0; i < 100; i++) {
      mjThreadPool_AddWork(tPool, Routine, NULL);
    }
    mjThreadPool_Delete(tPool);
    return 0;
}
