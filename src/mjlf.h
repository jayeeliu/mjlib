#ifndef __MJLF_H
#define __MJLF_H

#include <stdbool.h>
#include "mjthreadpool2.h"

struct mjLF {
    int             sfd;
    int             stop;
    mjThreadPool2   tPool; 
    mjProc          Routine;

    int             readTimeout;
    int             writeTimeout;
};
typedef struct mjLF* mjLF;

extern void mjLF_Run( mjLF srv );
extern bool mjLF_SetStop( mjLF srv, int value );
extern bool mjLF_SetTimeout( mjLF srv, int readTimeout, int writeTimeout );

extern mjLF mjLF_New( mjProc Routine, int maxThread, int sfd );
extern bool mjLF_Delete( mjLF srv );

#endif
