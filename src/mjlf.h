#ifndef _MJLF_H
#define _MJLF_H

#include <stdbool.h>
#include "mjthreadpool.h"

struct mjLF {
  int           sfd;            // server socket
  int           stop;
  mjthreadpool  tpool;          // thread pool 
  mjProc        Routine;

  int           read_timeout;   // read write timeout
  int           write_timeout;

  void*         private;        // private data
  mjProc        FreePrivate;
};
typedef struct mjLF* mjLF;

extern void mjLF_Run(mjLF srv);
extern bool mjLF_SetPrivate(mjLF srv, void* private, mjProc FreePrivate);
extern bool mjLF_SetStop(mjLF srv, int value);
extern bool mjLF_SetTimeout(mjLF srv, int read_timeout, int write_timeout);

extern mjLF mjLF_New(int sfd, mjProc Routine, int maxThread);
extern bool mjLF_Delete(mjLF srv);

#endif
