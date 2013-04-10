#define _SVID_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "mjpq.h"

#define LEN 50000

int main()
{
    mjPQ pq = mjPQ_New();
    if (!pq) {
        printf("mjprioq new error\n");
        return 1;
    }

    for (int i = 0; i < LEN; i++) {
        mjPQ_Insert(pq, random(), NULL);
    }

    for (int i = 0; i < LEN; i++) {
        printf("%lld\n", mjPQ_GetMinKey(pq));
        mjPQ_DelMin(pq);
    }

    mjPQ_Delete(pq);

    return 0;
}

