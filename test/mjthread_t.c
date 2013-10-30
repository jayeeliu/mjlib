#include <stdio.h>
#include <unistd.h>
#include "mjthread.h"

void* Routine(void* arg)
{
    static int count = 0;
    printf("count: %4d\n", count++);
    return NULL;
}

void* PreRoutine(void* arg)
{
    printf("Before thread\n");
    return NULL;
}

void* PostRoutine(void* arg)
{
    printf("After thread\n");
    return NULL;
}

int main() {
  mjthread thread = mjthread_new(PreRoutine, PostRoutine);
  for (int i = 0; i < 1000; i++) {
    mjthread_add_routine(thread, Routine, NULL);
  } 
  sleep(3);
  mjthread_delete(thread);

  return 0;
}
