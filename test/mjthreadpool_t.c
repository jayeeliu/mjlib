#include <stdio.h>
#include <unistd.h>
#include "mjthreadpool.h"

void* Routine(void* arg) {
    long a = 1;
    for(int i = 1; i < 20; i++) {
        a = a * i;
    }
    printf("%ld\n", a);
    return NULL;
}
 
int main()
{
    mjthreadpool tpool = mjthreadpool_new(10, NULL, NULL);

    for(int i = 0; i < 100; i++) {
      mjthreadpool_add_routine_plus(tpool, Routine, NULL);
    }
    mjthreadpool_delete(tpool);
    return 0;
}
