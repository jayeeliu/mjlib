#ifndef _MJEV_H
#define _MJEV_H

#include <stdbool.h>
#include <sys/epoll.h>
#include "mjproc.h"
#include "mjpq.h"
#include "mjlist.h"

#define MJEV_NONE       0
#define MJEV_READABLE   1
#define MJEV_WRITEABLE  2

// file event struct
typedef struct mjfevent {
    int     mask;
    mjProc  ReadCallBack;
    mjProc  WriteCallBack;
    void*   data;
} mjfevent;

// timer event struct
typedef struct mjtevent {
    int         valid;
    long long   time;
    mjProc      TimerProc;
    void*       data;
} mjtevent;

// pending proc to be run
typedef struct mjpending {
    mjProc              Proc;
    void*               data;
    struct list_head    pendingNode;
} mjpending;

#define MJEV_MAXFD          60000
#define PENDING_LIST_LEN    1024

// mjev struct
struct mjEV {
    int                 epfd;       // epoll fd
    mjfevent            fileEventList[MJEV_MAXFD];
    mjPQ                timerEventQueue;
    struct list_head    pendingHead;
};
typedef struct mjEV* mjEV;

extern bool         mjEV_Add( mjEV ev, int fd, int mask, mjProc Proc, void* data );
extern bool         mjEV_Del( mjEV ev, int fd, int mask );
extern mjtevent*    mjEV_AddTimer( mjEV ev, long long ms, mjProc Proc, void* data );
extern bool         mjEV_DelTimer( mjEV ev, mjtevent* te );
extern bool         mjEV_AddPending( mjEV ev, mjProc Proc, void* data );
extern bool         mjEV_DelPending( mjEV ev, void* data );
extern void         mjEV_Run( mjEV ev );

extern mjEV         mjEV_New();
extern bool         mjEV_Delete( mjEV ev );

#endif
