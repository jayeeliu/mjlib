#ifndef __MJEV2_H
#define __MJEV2_H

#include "mjlist.h"
#include "mjrbtree.h"
#include <stdbool.h>

#define MJEV2_NONE  0
#define MJEV2_READ  1
#define MJEV2_WRITE 2

#define MJEV2_MAXFD 60000

struct mjevt;
struct mjtevt;
struct mjev2;

typedef void (*mjevtProc)(struct mjev2*, struct mjevt*, void*);
typedef void (*mjtevtProc)(struct mjev2*, struct mjtevt*, void*);

struct mjtevt {
  long            _expire;
  mjtevtProc      _Handle;
  void*           _data;
  struct rb_node  _tnode;           // timer event node
};
typedef struct mjtevt* mjtevt;

struct mjevt {
  mjevtProc         _Handle;
  void*             _arg;
  struct mjtevt     _rtevt;           // read timer
  bool              _error;
  bool              _readReady;       // fd read ready
  bool              _writeReady;      // fd write ready
  bool              _readTimeout;     // read timeout
  bool              _writeTimeout;    // write timeout
  struct list_head  _rnode;           // link in _rhead
};
typedef struct mjevt* mjevt;

struct mjev2 {
  int               _epfd;
  struct mjevt      _events[MJEV2_MAXFD];
  struct list_head  _rhead;               // events which must be handle
  struct rb_root    _troot;               // timer event root
};
typedef struct mjev2* mjev2;

static inline mjevt mjevt_get(mjev2 ev2, int fd) {
  if (!ev2 || fd < 0 || fd > MJEV2_MAXFD) return NULL;
  return &ev2->_events[fd];
}

static inline bool mjevt_set_handle(mjevt evt, int mask, mjevtProc Handle, void* data) {
  if (!evt) return false;
  if (mask & MJEV2_READ) {
    evt->_ReadHandle = Handle;
    evt->_data = data;
  }
  if (mask & MJEV2_WRITE) {
    evt->_WriteHandle = Handle;
    evt->_data = data;
  }
}

extern bool   mjevt_ready(mjev2 ev2, mjevt evt, int mask);

extern bool   mjev2_add_event(mjev2 ev2, mjevt evt, int mask, long long ms);
extern bool   mjev2_del_event(mjev2 ev2, mjevt evt, int mask);
extern mjtevt mjev2_add_timer(mjev2 ev2, long long ms, mjtevtProc Handle, void* data);
extern bool   mjev2_mod_timer(mjev2 ev2, mjtevt tevt, long long ms, mjtevtProc Handle, void* data);
extern bool   mjev2_del_timer(mjev2 ev2, mjtevt tevt);
extern bool   mjev2_check_event(mjev2 ev2);
extern bool   mjev2_run_timer(mjev2 ev2);
extern bool   mjev2_run_event(mjev2 ev2);
extern mjev2  mjev2_new();
extern bool   mjev2_delete(mjev2 ev2);

static inline bool mjevt_read_timeout(mjevt evt) {
  if (!evt) return false;
  return evt->_readTimeout;
}

static inline bool mjevt_write_timeout(mjevt evt) {
  if (!evt) return false;
  return evt->_writeTimeout;
}

#endif
