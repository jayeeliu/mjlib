#ifndef _MJPROTO_HTTP
#define _MJPROTO_HTTP

#include "mjproc.h"
#include "mjhttpreq.h"
#include "mjhttprsp.h"
#include "mjstr.h"
#include "mjreg.h"

#define HTTP_GETREQ(conn) ((mjHttpData) conn->private)->request
#define HTTP_GETRSP(conn) ((mjHttpData) conn->private)->response
#define HTTP_GETPARAM(conn) ((mjHttpData) conn->private)->param

// per conn http data
struct mjhttpdata {
  mjhttpreq   req;
  mjstrlist   params;
  mjHttpRsp   rsp;
};
typedef struct mjhttpdata* mjhttpdata;

// url redirect struct
struct mjhttpurl {
  char    *url;
  mjProc  fun;
  mjreg   reg;
};

extern void*  http_mjlf_init(void* arg);
extern void*  http_mjlf_exit(void* arg);
extern void*  http_mjlf_routine(void* arg);

#endif
