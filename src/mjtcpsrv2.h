#ifndef __MJTCPSRV2_H
#define __MJTCPSRV2_H

#include "mjev.h"

#define MJTCPSRV_STANDALONE 0
#define MJTCPSRV_INNER      1

struct mjTcpSrv2 {
    int     sfd;            // socket, accept for standalone, read for inner
    int     stop;           // server stop
    int     type;           // tcpsrv type, standalone or inner
    mjEV    ev;             // event loop
    mjProc  Routine;        // server routine
    void*   mainServer;     // used in inner mode, point to mainServer
   
    void*   private;        // private data
    mjProc  FreePrivate; 
};
typedef struct mjTcpSrv2* mjTcpSrv2;

extern bool         mjTcpSrv2_Run( mjTcpSrv2 srv );
extern bool         mjTcpSrv2_SetPrivate( mjTcpSrv2 srv, void* private,
                            mjProc FreePrivate );
extern bool         mjTcpSrv2_SetStop( mjTcpSrv2 srv, int value );

extern mjTcpSrv2    mjTcpSrv2_New( int sfd, mjProc Routine, int type );
extern bool         mjTcpSrv2_Delete( mjTcpSrv2 srv );

#endif
