#ifndef __MJLF_H
#define __MJLF_H

#include <stdbool.h>
#include "mjthreadpool2.h"

struct mjLF {
    int             sfd;
    int             shutdown;
    mjThreadPool2   tPool; 
    mjProc          Routine;
};
typedef struct mjLF* mjLF;

extern void mjLF_Run( mjLF srv );
extern mjLF mjLF_New( mjProc Routine, int maxThread, int sfd );
extern bool mjLF_Delete( mjLF srv );

#endif
