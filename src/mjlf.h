#ifndef __MJLF_H
#define __MJLF_H

#include <stdbool.h>
#include "mjthreadpool.h"

struct mjLF {
    int             sfd;
    int             stop;
    mjThreadPool    tPool; 
    mjProc          Routine;

    int             readTimeout;        // read write timeout
    int             writeTimeout;

    void*           private;            // private data
    mjProc          FreePrivate;
};
typedef struct mjLF* mjLF;

extern void mjLF_Run( mjLF srv );
extern bool mjLF_SetPrivate( mjLF srv, void* private, mjProc FreePrivate );
extern bool mjLF_SetStop( mjLF srv, int value );
extern bool mjLF_SetTimeout( mjLF srv, int readTimeout, int writeTimeout );

extern mjLF mjLF_New( mjProc Routine, int maxThread, int sfd );
extern bool mjLF_Delete( mjLF srv );

#endif
