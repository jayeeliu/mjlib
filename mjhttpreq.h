#ifndef __MJHTTPREQ_H
#define __MJHTTPREQ_H

#include "mjstr.h"
#include "mjmap.h"

#define GET_METHOD      1
#define POST_METHOD     2
#define UNKNOWN_METHOD  5

struct mjHttpReq {
    int     methodType;     // method type
    mjStr   location;       // request location
    mjmap   parameter;      // request parameter
    mjmap   reqHeader;
};
typedef struct mjHttpReq* mjHttpReq;

extern mjHttpReq    mjHttpReq_New( mjStr data );
extern bool         mjHttpReq_Delete( mjHttpReq request );

#endif
