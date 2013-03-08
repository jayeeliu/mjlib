#ifndef _MJSOCK_H
#define _MJSOCK_H

extern int mjsock_tcpserver(int port);
extern int mjsock_accept(int fd);
extern int mjsock_SetBlocking( int fd, int blocking );
extern int mjsock_close(int fd);

#endif
