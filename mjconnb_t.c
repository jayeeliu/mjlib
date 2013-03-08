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

    int sfd = mjsock_tcpserver( 7879 );
    if ( sfd < 0 ) {
        printf( "Error to create socket" );
        return 1;
    }

    for ( ;; ) {
        int cfd = mjsock_accept( sfd );
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
        mjstr data = mjstr_new();
        mjConnB_ReadUntil(conn, "\r\n\r\n", data); 

        mjstr_copys(data, "END");
        mjConnB_Write(conn, data);
   
        mjstr_delete(data);
        mjConnB_Delete(conn);
  
        break;
    }

    close(sfd);

    return 0;
}
