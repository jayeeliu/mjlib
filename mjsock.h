#ifndef _MJSOCK_H
#define _MJSOCK_H

extern int mjSock_Accept( int fd );
extern int mjSock_SetBlocking( int fd, int blocking );
extern int mjSock_TcpServer( int port );
extern int mjSock_TcpSocket();
extern int mjSock_Close( int fd );

#endif
