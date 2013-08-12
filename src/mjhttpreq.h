#ifndef __MJHTTPREQ_H
#define __MJHTTPREQ_H

#include "mjstr.h"
#include "mjmap.h"

#define GET_METHOD      1
#define POST_METHOD     2
#define UNKNOWN_METHOD  5

struct mjhttpreq {
    int     method;         // method type
    mjStr   location;       // request location
    mjMap   parameter;      // request parameter
    mjMap   req_header;
};
typedef struct mjhttpreq* mjhttpreq;

extern mjhttpreq  mjhttpreq_new(mjStr data);
extern bool       mjhttpreq_delete(mjhttpreq request);

#endif
