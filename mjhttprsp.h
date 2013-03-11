#ifndef _MJHTTPRSP_H
#define _MJHTTPRSP_H

#include "mjmap.h"

struct mjHttpRsp {
    mjmap rspheader;
};
typedef struct mjHttpRsp* mjHttpRsp;

extern bool         mjHttpRsp_AddHeader( mjHttpRsp rsp, char* name, char* value );
extern mjStr        mjHttpRsp_HeaderToStr( mjHttpRsp rsp );

extern mjHttpRsp    mjHttpRsp_New();
extern bool         mjHttpRsp_Delete( mjHttpRsp rsp );

#endif
