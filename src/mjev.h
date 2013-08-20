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

struct mjfevent {
    int     _mask;
    mjProc  _ReadProc;
    mjProc  _WriteProc;
    void*   _arg;
};
typedef struct mjfevent* mjfevent;
// timer event struct
struct mjtevent {
    bool        _valid;
    long long   _time;
    mjProc      _TimerProc;
    void*       _arg;
};
typedef struct mjtevent* mjtevent;
// pending proc to be run
struct mjpending {
    mjProc              _PendingProc;
    void*               _arg;
    struct list_head    _pending_node;
};
typedef struct mjpending* mjpending;
// mjev struct
struct mjev {
    int                 _epfd;       // epoll fd
    struct mjfevent    	_file_event_list[MJEV_MAXFD];
    mjpq                _timer_event_queue;
    struct list_head    _pending_head;
};
typedef struct mjev* mjev;

// 3 types: file event/timer/pending
extern bool     mjev_add_fevent(mjev ev, int fd, int mask, mjProc Proc, void* arg);
extern bool     mjev_del_fevent(mjev ev, int fd, int mask);
extern mjtevent	mjev_add_timer(mjev ev, long long ms, mjProc TimerProc, void* arg);
extern bool     mjev_del_timer(mjev ev, mjtevent te);
extern bool     mjev_add_pending(mjev ev, mjProc PendingProc, void* arg);
extern bool   	mjev_del_pending(mjev ev, void* arg);
extern void     mjev_run(mjev ev);

extern mjev     mjev_new();
extern bool     mjev_delete(mjev ev);

#endif
