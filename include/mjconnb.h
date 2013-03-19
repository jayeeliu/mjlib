#ifndef _MJCONNB_H
#define _MJCONNB_H

#include "mjstr.h"
#include "mjev.h"

struct mjConnB {
    int             fd;                 // fd to control
    void*           server;             // server 
    mjStr           rbuf;               // read read buffer 

    int             readtype;           // read type
    const char*     delim;              // the delim when readtype is READUNTIL 
    int             rbytes;             // read data size when readtype is READBYTES 

    mjproc*         FreePrivate;        // free private data callback 
    void*           private;            // private data
};    
typedef struct mjConnB* mjConnB;

extern int      mjConnB_Read( mjConnB conn, mjStr data );
extern int      mjConnB_ReadBytes( mjConnB conn, mjStr data, int len );
extern int      mjConnB_ReadUntil( mjConnB conn, const char* delim, mjStr data );
extern int      mjConnB_Write( mjConnB conn, mjStr data );
extern int      mjConnB_WriteB( mjConnB conn, char* buf, int length );
extern int      mjConnB_WriteS( mjConnB conn, char* buf );

extern bool     mjConnB_SetPrivate( mjConnB conn, void* private, mjproc* FreePrivate );
extern bool     mjConnB_SetServer( mjConnB conn, void* server );
extern bool     mjConnB_SetTimeout( mjConnB conn, unsigned int readTimeout, 
                                unsigned int writeTimeout );

extern mjConnB  mjConnB_NewClient( const char* addr, int port, unsigned int timeout );
extern mjConnB  mjConnB_New( int fd );
extern void     mjConnB_Delete( mjConnB conn );

#endif
