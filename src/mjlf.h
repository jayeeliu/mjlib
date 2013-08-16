#ifndef _MJLF_H
#define _MJLF_H

#include <stdbool.h>
#include "mjthreadpool.h"
#include "mjmap.h"

struct mjlf {
  int           _sfd;             // server socket
  bool          _stop;
  mjthreadpool  _tpool;           // thread pool 
  mjProc        _Routine;         // run when new conn come

  void*         srv_init_arg;
  mjmap         _arg_map;

  int           _read_timeout;    // read write timeout
  int           _write_timeout;
};
typedef struct mjlf* mjlf;

extern void   mjlf_run(mjlf srv);
extern void*  mjlf_get_obj(mjlf srv, const char* key);
extern bool   mjlf_set_obj(mjlf srv, const char* key, void* obj, mjProc obj_free);
extern bool   mjlf_set_stop(mjlf srv, bool value);
extern bool   mjlf_set_timeout(mjlf srv, int read_timeout, int write_timeout);

extern mjlf mjlf_new(int sfd, mjProc Routine, int max_thread, mjProc Init_Srv,
    void* srv_init_arg, mjProc Init_Thrd, void* thrd_init_arg);
extern bool mjlf_delete(mjlf srv);

#endif
