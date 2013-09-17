#ifndef _MJSOCK_H
#define _MJSOCK_H

#include <stdbool.h>

extern int  mjsock_accept(int fd);
extern bool mjsock_set_blocking(int fd, int blocking);
extern int  mjsock_tcp_server(int port);
extern int  mjsock_tcp_socket();
extern int  mjsock_close(int fd);

#endif
