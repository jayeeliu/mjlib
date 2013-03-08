#include <stdio.h>
#include "mjev.h"

static int count = 0;
static int stop = 0;

void timershow(void *data)
{
    mjev ev = (mjev)data;
    printf("timer run: %4d\n", count++);
    if (count < 15) {
        mjEV_AddTimer(ev, 1000, timershow, ev);
    } else {
        stop = 1;
    }
}

int main()
{
    mjev ev = mjEV_New();
    if (!ev) {
        printf("mjev create error\n");
        return 1;
    }

    mjEV_AddTimer(ev, 1000, timershow, ev);
    while (!stop) {
        mjEV_Run(ev);
    }
    mjEV_Delete(ev);

    return 0;
}
