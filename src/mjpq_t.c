#define _SVID_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "mjpq.h"

#define LEN 5

int main()
{
    mjpq pq = mjpq_new();
    if (!pq) {
        printf("mjprioq new error\n");
        return 1;
    }

    struct pq_elt element[LEN];
    for (int i = 0; i < LEN; i++) {
        element[i].dt = random();
        element[i].data = NULL;
        mjpq_insert(pq, &element[i]);
    }

    struct pq_elt tmp;
    for (int i = 0; i < LEN; i++) {
        mjpq_min(pq, &tmp);
        mjpq_delmin(pq);
        printf("%lld\n", tmp.dt);
    }

    mjpq_delete(pq);

    return 0;
}

