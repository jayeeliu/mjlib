#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "mjsock.h"
#include "mjlog.h"

#define DEFAULT_BACKLOG	4096

/**
 * create noblock tcpserver 
 */
int mjSock_TcpServer(int port)
{
    int sfd, ret;
    int flags;
    struct linger ling = {0, 0};
    struct sockaddr_in saddr;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        MJLOG_ERR("socket create error");
        return -1;
    }

     //set socket option
    flags = 1;
    ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));
    if (ret < 0) {
        MJLOG_ERR("setsockopt REUSEADDR ERROR");
        close(sfd);
        return -1;
    }
   
    ret = setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&flags, sizeof(flags));
    if (ret < 0) {
        MJLOG_ERR("setsockopt KEEPALIVE ERROR");
        close(sfd);
        return -1;
    }

    ret = setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling));
    if (ret < 0) {
        MJLOG_ERR("setsockopt KEEPALIVE ERROR");
        close(sfd);
        return -1;
    } 

    //set socket address
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    //bind and listen
    ret = bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if(ret!=0) {
        MJLOG_ERR("bind error");
        close(sfd);
        return -1;
    }

    ret = listen(sfd, DEFAULT_BACKLOG);
    if (ret != 0) {
        MJLOG_ERR("listen error");
        close(sfd);
        return -1;
    }

    return sfd;
}

/* listen socket accept */
int mjSock_Accept(int sfd)
{
    struct sockaddr_in caddr;
    socklen_t caddr_len = 0;

    bzero(&caddr, sizeof(caddr));

    return accept(sfd, (struct sockaddr *)&caddr, &caddr_len);
}

/*
======================================================
mjSock_SetBlocking
    set socket to blocing
    return -1 -- failed, 0 -- success
======================================================
*/
int mjSock_SetBlocking( int fd, int blocking )
{
    int flags;
    if ( ( flags = fcntl( fd, F_GETFL, 0 ) ) < 0 ) {
        MJLOG_ERR( "fcntl F_GETFL error" );
        return -1;
    }

    if ( blocking ) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }

    if ( fcntl( fd, F_SETFL, flags ) < 0 ) {
        MJLOG_ERR( "fcntl F_SETFL error" );  
        return -1;
    }

    return 0;
}


int mjSock_Close(int sfd)
{
    return close(sfd);
}
