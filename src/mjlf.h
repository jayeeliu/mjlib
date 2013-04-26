#ifndef __MJLF_H
#define __MJLF_H

#include <stdbool.h>
#include "mjthreadpool2.h"

struct mjLF {
    mjThreadPool2   tPool; 
    mjProc          Routine;
    int             port;
};
typedef struct mjLF* mjLF;

extern mjLF mjLF_New( mjProc Routine, int maxThread, int port );
extern bool mjLF_Delete( mjLF server );

#endif
