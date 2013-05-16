#include <stdio.h>
#include <unistd.h>
#include "mjthreadpool.h"

static int count = 0;

void* fun(void* arg)
{
    printf("thread run[%lX]: %d\n", pthread_self(), count++);
    return NULL;
}

int main()
{
    mjThreadPool tpool = mjThreadPool_New(15);
    if (!tpool) {
        printf("mjthread pool create error\n");
        return 1;
    }

    for (int i = 0; i < 100; i++) {
        bool ret = mjThreadPool_AddWorker(tpool, fun, NULL);
        if ( !ret ) sleep(2);
    }
    sleep(5);
    mjThreadPool_Delete(tpool);
    return 0;
}
