#ifndef MJTCPSRV2_H
#define MJTCPSRV2_H

#include <stdbool.h>
#include "mjev2.h"
#include "mjthreadpool2.h"

struct mjTcpSrv2 {
    int     sfd;
    int     stop;
    mjEV2   ev;
    void*   mainServer;

    mjProc  Routine;
   
    void*   private;
    mjProc  FreePrivate; 
};
typedef struct mjTcpSrv2* mjTcpSrv2;

struct mjMainServer_AsyncData {
    int     finNotify_r;
    int     finNotify_w;
    mjEV2   ev;
    mjProc  workerRoutine;
    void*   rdata;
    mjProc  CallBack;
    void*   cdata;
};
typedef struct mjMainServer_AsyncData* mjMainServer_AsyncData;

#define MAX_SERVER_NUM  64

struct mjMainServer {
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
typedef struct mjMainServer* mjMainServer;

extern bool         mjMainServer_Async( mjMainServer srv, mjProc Routine, 
                        void* rdata, mjEV2 ev, mjProc CallBack, void* cdata );
extern bool         mjMainServer_Run( mjMainServer srv );
extern mjMainServer mjMainServer_New( int sfd, mjProc serverRoutine, 
                        int workerThreadNum );
extern bool         mjMainServer_Delete( mjMainServer srv );

#endif
