#ifndef _MJPROTO_HTTP
#define _MJPROTO_HTTP

#include "mjhttpreq.h"
#include "mjhttprsp.h"
#include "mjstr.h"
#include "mjreg.h"

#define HTTP_GETREQ( conn ) ( ( mjHttpData ) conn->private )->request
#define HTTP_GETRSP( conn ) ( ( mjHttpData ) conn->private )->response
#define HTTP_GETPARAM( conn ) ( ( mjHttpData ) conn->private )->param

// per conn http data
struct mjHttpData {
    mjHttpReq   request;
    mjHttpRsp   response;
    mjStrList   param;
};
typedef struct mjHttpData* mjHttpData;

// url redirect struct
struct mjHttpUrl {
    char*   url;
    void    ( *fun )( void* data );
    mjReg   reg;
};

extern void     http_Worker( void* arg );
extern void     http_InitSrv( void* arg );
extern void     http_ExitSrv( void* arg );
extern mjstr    FileToStr( const char* fileName );

#endif
