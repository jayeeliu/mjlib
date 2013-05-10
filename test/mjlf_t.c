#include <stdio.h>
#include <unistd.h>
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"

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
    int sfd = mjSock_TcpServer(7879);
    if ( sfd < 0 ) {
        printf( "mjSock_TcpServer error" );
        return 1;
    }
    
    mjLF server = mjLF_New( Routine, 20, sfd);
    if ( !server ) {
        printf( "mjLF_New error" );
        return 1;
    }
    mjLF_Run( server );
    mjLF_Delete( server );
    return 0;
}
