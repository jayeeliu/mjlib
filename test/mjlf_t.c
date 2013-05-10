#include <stdio.h>
#include <unistd.h>
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjopt2.h"

void* Routine( void* arg )
{
    mjConnB conn = ( mjConnB ) arg;
    mjStr data = mjStr_New();
    mjConnB_ReadUntil( conn, "\r\n\r\n", data );
    mjStr_Delete( data );
    mjConnB_WriteS( conn, "OK HERE\r\n" );
    mjConnB_Delete( conn );

    return NULL;
}

int main()
{
//    Daemonize();
    int port;
    int threadNum;

    mjOpt2_Define( NULL, "port", MJOPT_INT, &port, "7879" );
    mjOpt2_Define( NULL, "threadnum", MJOPT_INT, &threadNum, "20" );

    mjOpt2_ParseConf( "test.conf" );

    int sfd = mjSock_TcpServer(port);
    if ( sfd < 0 ) {
        printf( "mjSock_TcpServer error" );
        return 1;
    }
    
    mjLF server = mjLF_New( Routine, threadNum, sfd);
    if ( !server ) {
        printf( "mjLF_New error" );
        return 1;
    }
    mjLF_Run( server );
    mjLF_Delete( server );
    return 0;
}
