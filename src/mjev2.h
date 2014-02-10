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

typedef void (*mjevtProc)(struct mjev2*, struct mjevt*);
typedef void (*mjtevtProc)(struct mjev2*, struct mjtevt*);

struct mjtevt {
  long long       _expire;
  mjtevtProc      _Handle;
  void*           _data;
  struct rb_node  _tnode;           // timer event node
};
typedef struct mjtevt* mjtevt;

struct mjevt {
  int               _mask;            // which mask is set, read write or time
  mjevtProc         _ReadHandle;
  mjevtProc         _WriteHandle;
  void*             _data;
  struct mjtevt     _rtevt;           // read timer
  struct mjtevt     _wtevt;           // write timer
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

extern bool   mjev2_add_event(mjev2 ev2, int fd, int mask, mjevtProc Handle, void* data, long long ms);
extern bool   mjev2_del_event(mjev2 ev2, int fd, int mask);
extern mjtevt mjev2_add_timer(mjev2 ev2, long long ms, mjtevtProc Handle, void* data);
extern bool   mjev2_mod_timer(mjev2 ev2, mjtevt tevt, long long ms, mjtevtProc Handle, void* data);
extern bool   mjev2_del_timer(mjev2 ev2, mjtevt tevt);
extern bool   mjev2_check_event(mjev2 ev2);
extern bool   mjev2_run_timer(mjev2 ev2);
extern bool   mjev2_run_event(mjev2 ev2);
extern mjev2  mjev2_new();
extern bool   mjev2_delete(mjev2 ev2);

static inline void* mjevt_get_data(mjevt evt) {
  if (!evt) return NULL;
  return evt->_data;
}

static inline bool mjevt_read_timeout(mjevt evt) {
  if (!evt) return false;
  return evt->_readTimeout;
}

static inline bool mjevt_write_timeout(mjevt evt) {
  if (!evt) return false;
  return evt->_writeTimeout;
}

#endif
