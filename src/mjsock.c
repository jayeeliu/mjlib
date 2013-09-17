#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/*
===============================================================================
mjsock_tcp_socket
  create tcp socket
===============================================================================
*/
int mjsock_tcp_socket() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  return fd;
}

/*
===============================================================================
mjSock_TcpServer
    create tcpserver 
===============================================================================
*/
int mjsock_tcp_server(int port) {
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
  if (ret != 0) {
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
int mjsock_accept(int sfd) {
  struct sockaddr_in caddr;
  socklen_t caddr_len = 0;

  bzero(&caddr, sizeof(caddr));
  return accept(sfd, (struct sockaddr *)&caddr, &caddr_len);
}

/*
===============================================================================
mjSock_SetBlocking
    set socket to blocing
    return -1 -- failed, 0 -- success
===============================================================================
*/
bool mjsock_set_blocking(int fd, int blocking) {
  // get current flag
  int flags;
  if ((flags = fcntl(fd, F_GETFL, 0)) < 0) {
    MJLOG_ERR("fcntl F_GETFL error fd--%d  msg--%s", fd, strerror(errno));
    return false;
  }
  // set new flags;
  if (blocking) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  // set nonblock
  if (fcntl(fd, F_SETFL, flags) < 0) {
    MJLOG_ERR("fcntl F_SETFL error");  
    return false;
  }
  return true;
}


int mjsock_close(int sfd) {
  return close(sfd);
}
