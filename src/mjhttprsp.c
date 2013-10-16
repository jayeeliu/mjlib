#include "mjlog.h"
#include "mjhttprsp.h"
#include <stdio.h>
#include <stdlib.h>


// http response message
static const char* response[506];
static bool response_init = false;

static void mjhttprsp_init_response() {
  response[100] = "Continue";
  response[101] = "Switching Protocols";
  response[200] = "OK";
  response[201] = "Created";
  response[202] = "Accepted";
  response[203] = "Non-Authoritative Information";
  response[204] = "No Content";
  response[205] = "Reset Content";
  response[206] = "Partial Content";
  response[300] = "Multiple Choices";
  response[301] = "Moved Permanently";
  response[302] = "Found";
  response[303] = "See Other";
  response[304] = "Not Modified";
  response[305] = "Use Proxy";
  response[307] = "Temporary Redirect";
  response[400] = "Bad Request";
  response[401] = "Unauthorized";
  response[402] = "Payment Required";
  response[403] = "Forbidden";
  response[404] = "Not Found";
  response[405] = "Method Not Allowed";
  response[406] = "Not Acceptable";
  response[407] = "Proxy Authentication Required";
  response[408] = "Request Timeout";
  response[409] = "Conflict";
  response[410] = "Gone";
  response[411] = "Length Required";
  response[412] = "Precondition Failed";
  response[413] = "Request Entity Too Large";
  response[414] = "Request-URI Too Long";
  response[415] = "Unsupported Media Type";
  response[416] = "Requested Range Not Satisfiable";
  response[417] = "Expectation Failed";
  response[500] = "Internal Server Error";
  response[501] = "Not Implemented";
  response[502] = "Bad Gateway";
  response[503] = "Service Unavailable";
  response[504] = "Gateway Timeout";
  response[505] = "HTTP Version Not Supported";
}

/*
===============================================================================
mjhttprsp_AddHeader
    add header to http response
===============================================================================
*/
bool mjhttprsp_add_header(mjhttprsp rsp, char* name, char* value) {
  // sanity check
  if (!rsp) return false;
  // add value to header
  mjmap_set_strs(rsp->_header, name, value);
  return true;
}

/*
===============================================================================
mjhttprsp_HeaderToStr
    change http header to string, alloc and return mjstr
===============================================================================
*/
mjstr mjhttprsp_header_to_str(mjhttprsp rsp) {
  // alloc new mjstr
  mjstr str = mjstr_new(80);
  if (!str) {
    MJLOG_ERR("mjstr_New error");
    return NULL;
  }
  char buf[80];
  sprintf(buf, "HTTP/1.1 %d %s\r\n", rsp->_status, rsp->_response);
  mjstr_cats(str, buf);
  // iter the mjmap
  mjitem item = mjmap_get_next(rsp->_header, NULL);
  while (item) {
    mjstr_cat(str, item->key);
    mjstr_cats(str, ": ");
    mjstr_cat(str, item->obj); 
    mjstr_cats(str, "\r\n");
    item = mjmap_get_next(rsp->_header, item);
  }
  return str;
}

bool mjhttprsp_set_status(mjhttprsp rsp, unsigned int status) {
  if (!rsp || status > 505 || !response[status]) return false;
  rsp->_status = status;
  rsp->_response = response[status];
  return true;
}

/*
===============================================================================
mjhttprsp_New
    create new mjhttprsp struct
===============================================================================
*/
mjhttprsp mjhttprsp_new() {
  if (!response_init) mjhttprsp_init_response();
  // alloc mjhttprsp struct
  mjhttprsp rsp = (mjhttprsp) calloc(1, sizeof(struct mjhttprsp));
  if (!rsp) {
    MJLOG_ERR("calloc error");
    return NULL;
  }
  // alloc new mjMap
  rsp->_header = mjmap_new(131);
  if (!rsp->_header) {
    MJLOG_ERR("mjmap new error");
    free(rsp);
    return NULL;
  }
  return rsp;
}

/*
===============================================================================
mjhttprsp_Delete
    delete mjhttprsp struct
===============================================================================
*/
bool mjhttprsp_delete(mjhttprsp rsp) {
    // sanity check
    if (!rsp) return false;
    // relase rspHeader
    if (rsp->_header) mjmap_delete(rsp->_header);
    free(rsp);
    return true;
}
