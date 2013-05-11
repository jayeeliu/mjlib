#include <stdio.h>
#include <unistd.h>
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjopt2.h"
#include "mjproto_txt.h"

PROTO_TXT_ROUTINE routineList[] = {
  {"get", NULL},
  {"put", NULL},
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
