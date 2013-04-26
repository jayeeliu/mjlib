#ifndef __MJLF_H
#define __MJLF_H

#include "mjthreadpool2.h"

struct mjLF {
    mjThreadPool2   tPool; 
    mjProc          Routine;
};
typedef struct mjLF* mjLF;

#endif
