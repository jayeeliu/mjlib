#ifndef __MJEV2_H
#define __MJEV2_H

#define MJEV2_NONE  0
#define MJEV2_READ  1
#define MJEV2_WRITE 2

#define MJEV2_MAXFD 60000

struct mjevt;
typedef struct mjevt* (*mjevtProc)(struct mjevt*);

struct mjevt {
  int               _fd;              // fd
  int               _mask;            // which mask is set in epoll
  mjevtProc         _ReadHandle;
  mjevtProc         _WriteHandle;
  mjevtProc         _TimeoutHandle;
  void*             _data;
  struct list_head  _pnode;
  long long         _expire;
  struct rb_node    _tnode;           // timer event node
  bool              _closed;
  bool              _error;
  bool              _readReady;
  bool              _writeReady;
  bool              _timeout;
};
typedef struct mjevt mjevt;

struct mjev2 {
  int               _epfd;
  struct mjevt      _events[MJEV2_MAXFD];
  struct list_head  _phead;
  struct list_head  _fhead;               // free node head, for timer events
  struct rb_root    _troot;               // timer event root
};
typedef struct mjev2 mjev2;

extern bool   mjev2_add_event(mjev2 ev2, int fd, int mask, mjevtHandle Handle, void* data);
extern bool   mjev2_del_event(mjev2 ev2, int fd, int mask);
extern mjevt  mjev2_add_timer(mjev2 ev2, long long ms, mjevtHandle Handle, void* data);
extern bool   mjev2_del_timer(mjev2 ev2, mjevt evt);
extern mjev2  mjev2_new();
extern bool   mjev2_delete(mjev2 ev2);

#endif
