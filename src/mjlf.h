#ifndef _MJLF_H
#define _MJLF_H

#include <stdbool.h>
#include "mjthreadpool.h"

struct mjlf {
  int           sfd;            // server socket
  int           stop;
  mjthreadpool  tpool;          // thread pool 
  mjProc        Routine;

  int           read_timeout;   // read write timeout
  int           write_timeout;
};
typedef struct mjlf* mjlf;

extern void mjlf_run(mjlf srv);
extern bool mjlf_set_stop(mjlf srv, int value);
extern bool mjlf_set_timeout(mjlf srv, int read_timeout, int write_timeout);

extern mjlf mjlf_new(int sfd, mjProc Routine, int maxThread,
  mjProc Init_Routine, void* init_arg, mjProc Exit_Routine);
extern bool mjlf_delete(mjlf srv);

#endif
