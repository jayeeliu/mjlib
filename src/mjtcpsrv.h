#ifndef __MJTCPSRV2_H
#define __MJTCPSRV2_H

#include "mjev.h"

#define MJTCPSRV_STANDALONE 0
#define MJTCPSRV_INNER      1

struct mjTcpSrv {
    int     sfd;            // socket, accept for standalone, read for inner
    int     stop;           // server stop
    int     type;           // tcpsrv type, standalone or inner
    mjEV    ev;             // event loop
    mjProc  Routine;        // server routine
    void*   mainServer;     // used in inner mode, point to mainServer

    mjProc  InitSrv;        // init Server proc
    mjProc  ExitSrv;        // exit Server proc
   
    void*   private;        // private data
    mjProc  FreePrivate; 
};
typedef struct mjTcpSrv* mjTcpSrv;

extern void*        mjTcpSrv_AcceptRoutine( void* arg );
extern void*        mjTcpSrv_Run( void* arg );
extern bool         mjTcpSrv_SetPrivate( mjTcpSrv srv, void* private,
                            mjProc FreePrivate );
extern bool         mjTcpSrv_SetSrvProc( mjTcpSrv srv, mjProc InitSrv,
                            mjProc ExitSrv );
extern bool         mjTcpSrv_SetStop( mjTcpSrv srv, int value );

extern mjTcpSrv     mjTcpSrv_New( int sfd, mjProc Routine, int type );
extern void*        mjTcpSrv_Delete( void* arg );

#endif
