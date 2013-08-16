#ifndef _MJLF_H
#define _MJLF_H

#include <stdbool.h>
#include "mjthreadpool.h"

struct mjlf {
  int           _sfd;             // server socket
  bool          _stop;
  mjthreadpool  _tpool;           // thread pool 
  mjProc        _Routine;         // run when new conn come

  int           _read_timeout;    // read write timeout
  int           _write_timeout;
};
typedef struct mjlf* mjlf;

extern void mjlf_run(mjlf srv);
extern bool mjlf_set_stop(mjlf srv, bool value);
extern bool mjlf_set_timeout(mjlf srv, int read_timeout, int write_timeout);

extern mjlf mjlf_new(int sfd, mjProc Routine, int max_thread, mjProc Init, 
    void* init_arg);
extern bool mjlf_delete(mjlf srv);

#endif
