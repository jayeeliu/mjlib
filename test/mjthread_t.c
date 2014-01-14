#include <stdio.h>
#include <unistd.h>
#include "mjthread.h"

static void* Routine(void* arg) {
  static int count = 0;
  printf("count: %4d\n", count++);
  return NULL;
}

static void* CBRoutine(void* arg) {
  printf("cb routine run\n");
  return NULL;
}

int main() {
  mjthread thread = mjthread_new();
  mjthread_run(thread);
  for (int i = 0; i < 1000; i++) {
    mjthread_add_task(thread, Routine, NULL);
    mjthread_set_cb(thread, CBRoutine, NULL);
  } 
  sleep(3);
  mjthread_delete(thread);

  return 0;
}
