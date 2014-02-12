#ifndef _MJSOCK_H
#define _MJSOCK_H

#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

extern int  mjsock_accept(int fd);
extern int  mjsock_accept_timeout(int fd, int timeout);
extern bool mjsock_set_blocking(int fd, int blocking);
extern int  mjsock_tcp_server(int port);


static inline int mjsock_tcp_socket() {
  return socket(AF_INET, SOCK_STREAM, 0);
}

static inline int mjsock_udp_socket() {
  return socket(AF_INET, SOCK_DGRAM, 0);
}

static inline int mjsock_close(int sfd) {
  return close(sfd);
}

#endif
