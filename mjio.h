#ifndef _MJIO_H
#define _MJIO_H

#include "mjstr.h"

struct mjIO {
    const char* fileName;
    int         fd;
    mjStr       buffer;
};
typedef struct mjIO* mjIO;

extern int  mjIO_Read( mjIO io, mjStr data, int len );
extern int  mjIO_ReadLine( mjIO io, mjStr data );
extern mjIO mjIO_New( const char* fileName );
extern bool mjIO_Delete( mjIO io );

#endif
