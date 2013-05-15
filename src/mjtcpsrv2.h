#ifndef MJTCPSRV2_H
#define MJTCPSRV2_H

#include <stdbool.h>
#include "mjev2.h"
#include "mjthreadpool2.h"

struct mjTcpSrv2 {
    int     sfd;
    int     stop;
    mjEV2   ev;

    mjProc  Routine;
   
    void*   private;
    mjProc  FreePrivate; 
};
typedef struct mjTcpSrv2* mjTcpSrv2;

#define MAX_SERVER_NUM  64

struct mjServer {
    int             sfd;
    int             stop;
    
    int             workerThreadNum;
    mjThreadPool2   workerThreadPool;

    mjProc          serverRoutine;
    int             serverNum;
    int             serverNotify[MAX_SERVER_NUM];
    mjTcpSrv2       server[MAX_SERVER_NUM];
    mjThread        serverThread[MAX_SERVER_NUM];
};
typedef struct mjServer* mjServer;

extern bool     mjServer_Async( mjServer srv, mjProc Routine, void* rdata, 
                    mjEV2 ev, mjProc CallBack, void* cdata );
extern bool     mjServer_Run( mjServer srv );

extern mjServer mjServer_New( int sfd, mjProc workerRoutine );
extern bool     mjServer_Delete( mjServer srv );

#endif
