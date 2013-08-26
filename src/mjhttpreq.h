#ifndef __MJHTTPREQ_H
#define __MJHTTPREQ_H

#include "mjstr2.h"
#include "mjmap.h"

#define NONE_METHOD     0
#define GET_METHOD      1
#define POST_METHOD     2
#define UNKNOWN_METHOD  5

struct mjhttpreq {
    int     method;         // method type
    mjstr   location;       // request location
    mjmap   req_header;
};
typedef struct mjhttpreq* mjhttpreq;

extern mjhttpreq  mjhttpreq_new();
extern bool       mjhttpreq_init(mjhttpreq req, mjstr data);
extern bool       mjhttpreq_delete(mjhttpreq req);

#endif
