#ifndef _MJPROTO_HTTP
#define _MJPROTO_HTTP

#include "mjproc.h"
#include "mjhttpreq.h"
#include "mjhttprsp.h"
#include "mjstr.h"
#include "mjreg.h"

// per conn http data
struct mjhttpdata {
  mjhttpreq req;
  mjhttprsp rsp;
  mjslist   params;
};
typedef struct mjhttpdata* mjhttpdata;

// url redirect struct
struct mjhttpurl {
  char*   url;
  mjProc  fun;
  mjreg   reg;
};

#endif
