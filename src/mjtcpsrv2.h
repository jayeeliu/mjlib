#ifndef MJTCPSRV2_H
#define MJTCPSRV2_H

#include <stdbool.h>
#include "mjev.h"

struct mjTcpSrv2 {
    int     sfd;
    int     stop;
    mjev    ev;

    mjProc  Routine;
   
    void*   private;
    mjProc  FreePrivate; 
};
typedef struct mjTcpSrv2* mjTcpSrv2;

#endif
