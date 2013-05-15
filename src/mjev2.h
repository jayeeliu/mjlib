#ifndef _MJEV2_H
#define _MJEV2_H

#include <stdbool.h>
#include <sys/epoll.h>
#include "mjproc.h"
#include "mjpq.h"
#include "mjlist.h"

#define MJEV_NONE       0
#define MJEV_READABLE   1
#define MJEV_WRITEABLE  2

// file event struct
typedef struct mjfevent2 {
    int     mask;
    mjProc  ReadCallBack;
    mjProc  WriteCallBack;
    void*   data;
} mjfevent2;

// timer event struct
typedef struct mjtevent2 {
    int         valid;
    long long   time;
    mjProc      TimerProc;
    void*       data;
} mjtevent2;

// pending proc to be run
typedef struct mjpending2 {
    mjProc              Proc;
    void*               data;
    struct list_head    pendingNode;
} mjpending2;

#define MJEV_MAXFD          60000
#define PENDING_LIST_LEN    1024

// mjev struct
struct mjEV2 {
    int                 epfd;       // epoll fd
    mjfevent2           fileEventList[MJEV_MAXFD];
    mjPQ                timerEventQueue;
    struct list_head    pendingHead;
};
typedef struct mjEV2* mjEV2;

extern bool         mjEV2_Add( mjEV2 ev, int fd, int mask, mjProc Proc, void* data );
extern bool         mjEV2_Del( mjEV2 ev, int fd, int mask );
extern mjtevent2*   mjEV2_AddTimer( mjEV2 ev, long long ms, mjProc Proc, void* data );
extern bool         mjEV2_DelTimer( mjEV2 ev, mjtevent2* te );
extern bool         mjEV2_AddPending( mjEV2 ev, mjProc Proc, void* data );
extern bool         mjEV2_DelPending( mjEV2 ev, void* data );
extern void         mjEV2_Run( mjEV2 ev );

extern mjEV2        mjEV2_New();
extern bool         mjEV2_Delete( mjEV2 ev );

#endif
