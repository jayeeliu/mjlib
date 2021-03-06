#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mjthreadpool.h"

void* Routine(mjthread thread, void* arg) {
    int* value = arg;

    long a = 1;
    for(int i = 1; i < 20; i++) {
        a = a * i;
    }
    printf("%d show\n", *value);
    free(value);
    return NULL;
}
 
int main()
{
    mjthreadpool tpool = mjthreadpool_new(2);

    mjthreadpool_run(tpool);
    for(int i = 0; i < 100; i++) {
      int* num = (int*) malloc(sizeof(int));
      *num = i;
      mjthreadpool_set_task(tpool, Routine, num);
    }
    mjthreadpool_delete(tpool);
    return 0;
}
