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

#define MJEV_MAXFD          60000

struct mjfevent {
  int     _mask;  // event mask
  mjProc  _RRT;   // read routine
  mjProc  _WRT;   // write routine
  void*   _arg;   // routine arg
};
typedef struct mjfevent* mjfevent;
// timer event struct
struct mjtevent {
  bool      _valid; // tevent valid ?
  long long _time;
  mjProc    _TRT;   // timer routine
  void*     _arg;   // timer routine arg
};
typedef struct mjtevent* mjtevent;
// pending proc to be run
struct mjpending {
  mjProc            _PRT;   // pending routine
  void*             _arg;   // pending arg
  struct list_head  _pnode; // pending node
};
typedef struct mjpending* mjpending;
// mjev struct
struct mjev {
  int               _epfd;                    // epoll fd
  struct mjfevent   _fevent_list[MJEV_MAXFD]; // fevent list
  mjpq              _tevent_queue;            // timer event queue
  struct list_head  _phead;                   // pending head
};
typedef struct mjev* mjev;

// 3 types: file event/timer/pending
extern bool     mjev_add_fevent(mjev ev, int fd, int mask, mjProc RT, void* arg);
extern bool     mjev_del_fevent(mjev ev, int fd, int mask);
extern mjtevent mjev_add_timer(mjev ev, long long ms, mjProc TRT, void* arg);
extern bool     mjev_del_timer(mjev ev, mjtevent te);
extern bool     mjev_add_pending(mjev ev, mjProc PRT, void* arg);
extern bool     mjev_del_pending(mjev ev, void* arg);
extern void     mjev_run(mjev ev);

extern mjev     mjev_new();
extern bool     mjev_delete(mjev ev);

#endif
