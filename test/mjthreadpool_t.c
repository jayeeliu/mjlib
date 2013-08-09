#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mjthreadpool.h"

void* Routine(void* arg) {
    mjthread thread = (mjthread) arg;
    long a = 1;
    for(int i = 1; i < 20; i++) {
        a = a * i;
    }
    printf("%d show\n", *(int*)thread->arg);
    return NULL;
}
 
int main()
{
    mjthreadpool tpool = mjthreadpool_new(2, NULL, NULL);

    for(int i = 0; i < 100; i++) {
      int* num = (int*) malloc(sizeof(int));
      *num = i;
      mjthreadpool_add_routine_plus(tpool, Routine, num);
    }
    mjthreadpool_delete(tpool);
    return 0;
}
