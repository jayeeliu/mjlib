#ifndef MJTCPSRV2_H
#define MJTCPSRV2_H

#include "mjthreadpool2.h"

struct mjTcpSrv2 {
    int             sfd;
    mjThreadPool2   tPool;
};
typedef struct mjTcpSrv2* mjTcpSrv2;

#endif
