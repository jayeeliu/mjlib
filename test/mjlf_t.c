#include <stdio.h> 
#include <unistd.h>
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjopt2.h"
#include "mjproto_txt.h"

void* GetRoutine(void* arg)
{
  struct mjProtoTxtData* cmdData = (struct mjProtoTxtData*) arg;
  mjConnB_WriteS(cmdData->conn, "Get Called\r\n");
  return NULL; 
} 

void* PutRoutine(void* arg)
{
  struct mjProtoTxtData* cmdData = (struct mjProtoTxtData*) arg;
  mjConnB_WriteS(cmdData->conn, "Put Called\r\n");
  return NULL;
}

void* StatRoutine(void* arg)
{
  struct mjProtoTxtData* cmdData = (struct mjProtoTxtData*) arg;
  mjConnB_WriteS(cmdData->conn, "OK Here\r\n");
  return NULL;
}

PROTO_TXT_ROUTINE routineList[] = {
  {"get", GetRoutine},
  {"put", PutRoutine},
  {"stat", StatRoutine},
};

void* Routine( void* arg )
{
    mjConnB conn = ( mjConnB ) arg;
    mjTxt_RunCmd( routineList, sizeof(routineList) / sizeof(PROTO_TXT_ROUTINE), conn ); 
    mjConnB_Delete( conn );
    return NULL;
}

int main()
{
    int port;
    int threadNum;

    mjOpt2_Define( NULL, "port", MJOPT_INT, &port, "7879" );
    mjOpt2_Define( NULL, "threadnum", MJOPT_INT, &threadNum, "20" );
    mjOpt2_ParseConf( "test.conf" );

    int sfd = mjSock_TcpServer( port );
    if ( sfd < 0 ) {
        printf( "mjSock_TcpServer error" );
        return 1;
    }
    
    mjLF server = mjLF_New( Routine, threadNum, sfd );
    if ( !server ) {
        printf( "mjLF_New error" );
        return 1;
    }
    mjLF_Run( server );
    mjLF_Delete( server );
    return 0;
}
