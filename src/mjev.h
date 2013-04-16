#ifndef _MJEV_H
#define _MJEV_H

#include <stdbool.h>
#include <sys/epoll.h>
#include "mjproc.h"
#include "mjpq.h"

#define MJEV_MAXFD      60000

#define MJEV_NONE       0
#define MJEV_READABLE   1
#define MJEV_WRITEABLE  2

// file event struct
typedef struct mjfevent {
    int     mask;
    mjProc  ReadProc;
    mjProc  WriteProc;
    void*   data;
} mjfevent;

// timer event struct
typedef struct mjtevent {
    int         valid;
    long long   time;
    mjProc      TimerProc;
    void*       data;
} mjtevent;

// mjev struct
struct mjev {
    int         epfd;   // epoll fd
    mjfevent    fileEventList[MJEV_MAXFD];
    mjPQ        timerEventQueue;
};
typedef struct mjev* mjev;

extern bool         mjEV_Add( mjev ev, int fd, int mask, mjProc proc, void* data );
extern bool         mjEV_Del( mjev ev, int fd, int mask );
extern mjtevent*    mjEV_AddTimer( mjev ev, long long ms, mjProc proc, void* data );
extern bool         mjEV_DelTimer( mjev ev, mjtevent *te );
extern void         mjEV_Run( mjev ev );

extern mjev         mjEV_New();
extern void         mjEV_Delete( mjev ev );

#endif
