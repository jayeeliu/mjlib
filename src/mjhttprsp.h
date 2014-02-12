#ifndef _MJHTTPRSP_H
#define _MJHTTPRSP_H

#include "mjmap.h"

struct mjhttprsp {
  unsigned int  _status;    // http status code
  const char*   _response;  // http response string
  mjmap         _header;    // response header
  mjstr         _content;
};
typedef struct mjhttprsp* mjhttprsp;

extern bool       mjhttprsp_set_status(mjhttprsp rsp, unsigned int status);
extern bool       mjhttprsp_add_header(mjhttprsp rsp, char* name, char* value);
extern mjstr      mjhttprsp_to_str(mjhttprsp rsp);
extern mjhttprsp  mjhttprsp_new();
extern bool       mjhttprsp_delete(mjhttprsp rsp);

static inline bool mjhttprsp_set_str(mjhttprsp rsp, mjstr str) {
  if (!rsp) return false;
  mjstr_copy(rsp->_content, str);
  return true;
}

static inline bool mjhttprsp_set_strs(mjhttprsp rsp, char* str) {
  if (!rsp) return false;
  mjstr_copys(rsp->_content, str);
  return true;
}

#endif
