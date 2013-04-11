#include <stdio.h>
#include "mjtcpsrvtp.h"
#include "mjsock.h"

int main()
{
    int sfd = mjSock_TcpServer( 7879 );

    mjTcpSrvTP srv = mjTcpSrvTP_New( sfd, 10 );
    if ( !srv ) {
        printf( "Error create server" );
        return 1;
    }

    mjTcpSrvTP_Run( srv );

    mjTcpSrvTP_Delete( srv );

    return 0;
}
