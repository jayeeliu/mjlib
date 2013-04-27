#ifndef __MJLF_H
#define __MJLF_H

#include <stdbool.h>
#include "mjthreadpool2.h"

struct mjLF {
    int             sfd;
    mjThreadPool2   tPool; 
    mjProc          Routine;
};
typedef struct mjLF* mjLF;

extern mjLF mjLF_New( mjProc Routine, int maxThread, int sfd );
extern bool mjLF_Delete( mjLF server );

#endif
