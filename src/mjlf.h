#ifndef _MJLF_H
#define _MJLF_H

#include <stdbool.h>
#include "mjthreadpool2.h"
#include "mjmap.h"

struct mjlf {
  int           _sfd;             // server socket
  bool          _stop;
  mjthreadpool  _tpool;           // thread pool 
  mjProc        _Routine;         // run when new conn come

  void*         iarg;
  mjmap         _arg_map;

  int           _rto;    // read write timeout
  int           _wto;
};
typedef struct mjlf* mjlf;

extern void   mjlf_run(mjlf srv);
extern void*  mjlf_get_obj(mjlf srv, const char* key);
extern bool   mjlf_set_obj(mjlf srv, const char* key, void* obj, mjProc obj_free);
extern bool   mjlf_set_stop(mjlf srv, bool value);
extern bool   mjlf_set_timeout(mjlf srv, int read_timeout, int write_timeout);

extern mjlf mjlf_new(int sfd, mjProc Routine, int max_thread, mjProc Init_Srv,
    void* s_arg, mjProc Init_Thrd, void* t_arg);
extern bool mjlf_delete(mjlf srv);

#endif
