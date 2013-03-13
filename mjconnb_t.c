#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "mjconnb.h"
#include "mjsock.h"
#include "mjcomm.h"

void wait_child(int signo)
{
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0);
}

int main()
{
    signal(SIGCHLD, wait_child);

    int sfd = mjSock_TcpServer( 7879 );
    if ( sfd < 0 ) {
        printf( "Error to create socket" );
        return 1;
    }

    for ( ;; ) {
        int cfd = mjSock_Accept( sfd );
        if ( cfd < 0 ) {
            printf("Some Error Happened\n");
            break;
        }

        pid_t pid = fork();
        if (pid < 0) {
            break;
        }
        if (pid != 0) {
            close(cfd);
            continue;
        }
        
        mjConnB conn = mjConnB_New( cfd );
        if (conn == NULL) {
            printf("can't create mjConnB struct\n");
            break;
        }
        mjStr data = mjStr_New();
        mjConnB_ReadUntil(conn, "\r\n\r\n", data); 

        mjStr_CopyS(data, "END");
        mjConnB_Write(conn, data);
   
        mjStr_Delete(data);
        mjConnB_Delete(conn);
  
        break;
    }

    close(sfd);

    return 0;
}
