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
#define MJEV_MAXFD          60000

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
// mjev struct
struct mjev {
    int                 epfd;       // epoll fd
    mjfevent            fileEventList[MJEV_MAXFD];
    mjpq                timerEventQueue;
    struct list_head    pendingHead;
};
typedef struct mjev* mjev;

// 3 types: file event/timer/pending
extern bool         mjev_add_fevent(mjev ev, int fd, int mask, mjProc Proc, void* data);
extern bool         mjev_del_fevent(mjev ev, int fd, int mask);
extern mjtevent*    mjev_add_timer(mjev ev, long long ms, mjProc Proc, void* data);
extern bool         mjev_del_timer(mjev ev, mjtevent* te);
extern bool         mjev_add_pending(mjev ev, mjProc Proc, void* data);
extern bool         mjev_del_pending(mjev ev, void* data);
extern void         mjev_run(mjev ev);

extern mjev         mjev_new();
extern bool         mjev_delete(mjev ev);

#endif
