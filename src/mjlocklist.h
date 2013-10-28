#ifndef __MJLOCKLIST_H
#define __MJLOCKLIST_H

#include "mjproc.h"
#include "mjlist.h"
#include <pthread.h>

struct mjlocklist {
  pthread_mutex_t   _lock;
  struct list_head  _list;
  mjProc            _FREE;
};
typedef struct mjlocklist* mjlocklist;

#endif
