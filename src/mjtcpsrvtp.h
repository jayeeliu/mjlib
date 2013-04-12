#ifndef _MJTCPSRVTP_H
#define _MJTCPSRVTP_H

#include <stdbool.h>
#include "mjthreadpool.h"

struct mjTcpSrvTP {
    int             sfd;
    int             stop;
    mjThreadPool    tpool;
    mjthread*       Handler;
};
typedef struct mjTcpSrvTP* mjTcpSrvTP;

extern bool mjTcpSrvTP_Run( mjTcpSrvTP srv );
extern bool mjTcpSrvTP_SetHandler( mjTcpSrvTP srv, mjthread* Handler );

extern mjTcpSrvTP   mjTcpSrvTP_New( int sfd, int threadNum );
extern bool         mjTcpSrvTP_Delete( mjTcpSrvTP srv );

#endif