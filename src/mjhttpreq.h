#ifndef __MJHTTPREQ_H
#define __MJHTTPREQ_H

#include "mjstr.h"
#include "mjmap.h"

#define INVALID_METHOD  0
#define GET_METHOD      1
#define HEAD_METHOD     2
#define PUT_METHOD      3
#define DELETE_METHOD   4
#define POST_METHOD     5
#define OPTIONS_METHOD  6

struct mjhttpreq {
    int     method;   // method type
    mjstr   _path;    // request path
    mjstr   _query;   // request query without ?
    char*   _version; // http version
    mjmap   _header;  // request header
    mjslist _htmp;    // used for header parse
    mjslist _ftmp;    // user for header field parse
};
typedef struct mjhttpreq* mjhttpreq;

extern mjhttpreq  mjhttpreq_new();
extern bool       mjhttpreq_parse(mjhttpreq req, mjstr data);
extern bool       mjhttpreq_delete(mjhttpreq req);

#endif
