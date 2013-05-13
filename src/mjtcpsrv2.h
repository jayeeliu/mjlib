#ifndef MJTCPSRV2_H
#define MJTCPSRV2_H

#include <stdbool.h>
#include "mjev.h"
#include "mjthread.h"

struct mjTcpSrv2 {
    int     sfd;
    int     stop;
    mjev    ev;

    mjProc  Routine;
   
    void*   private;
    mjProc  FreePrivate; 
};
typedef struct mjTcpSrv2* mjTcpSrv2;

#define MAX_SERVER_NUM  64

struct mjServer {
    int         sfd;
    int         stop;
    mjProc      Routine;
    int         serverNum;
    int         serverNotify[MAX_SERVER_NUM];
    mjTcpSrv2   server[MAX_SERVER_NUM];
    mjThread    thread[MAX_SERVER_NUM];
};
typedef struct mjServer* mjServer;

extern bool     mjServer_Run( mjServer srv );

extern mjServer mjServer_New( int sfd, mjProc Routine );
extern bool     mjServer_Delete( mjServer srv );

#endif
