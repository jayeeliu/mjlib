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
/*
struct mjHttpData {
  mjHttpReq   request;
  mjHttpRsp   response;
  mjStrList   param;
};
typedef struct mjHttpData* mjHttpData;
*/

// url redirect struct
struct mjhttpurl {
  char    *url;
  mjProc  fun;
  mjreg   reg;
};

/*
extern void*  http_Worker(void *arg);
extern void*  http_InitTcpSrv(void *arg);
extern void*  http_InitMainSrv(void *arg);
extern void*  http_ExitTcpSrv(void *arg);
extern void*  http_ExitMainSrv(void *arg);
extern mjStr  FileToStr(const char *fileName);
*/
extern void*  http_mjlf_init(void* arg);
extern void*  http_mjlf_exit(void* arg);
extern void*  http_mjlf_routine(void* arg);

#endif
