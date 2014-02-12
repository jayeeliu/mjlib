#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "mjhttpreq.h"
#include "mjlog.h"

#define MAX_HEADER_LEN  20
#define MAX_FIELD_LEN   10


/*
===============================================================================
mjhttpreq_new
  create new mjhttpreq struct
===============================================================================
*/
mjhttpreq mjhttpreq_new() {
  // create mjhttpreq struct
  mjhttpreq req = (mjhttpreq) calloc(1, sizeof(struct mjhttpreq));
  if (!req) {
    MJLOG_ERR("mjhttpreq alloc error");
    return NULL;
  }
  // alloc location string
  req->_path = mjstr_new(80);
	req->_query = mjstr_new(80);
  req->_header = mjmap_new(131);
	req->_htmp = mjslist_new();
	req->_ftmp	= mjslist_new();
  if (!req->_path || !req->_query || !req->_header || 
      !req->_htmp || !req->_ftmp) {
    MJLOG_ERR("mjhttpreq init error");
		mjhttpreq_delete(req);
    return NULL;
  }
  return req;
}

/*
===============================================================================
mjhttpreq_init
  create new mjhttpreq struct
===============================================================================
*/
bool mjhttpreq_parse(mjhttpreq req, mjstr data) {
  // sanity check
  if (!req || !data) return false;  
	mjslist header = req->_htmp;
	mjslist field = req->_ftmp;
  // clean header and field first
  mjslist_clean(header);
  mjslist_clean(field);
  mjstr_split(data, "\r\n", header);
  mjstr_split(header->data[0], " ", field);
  // check cmd length
  if (field->len != 3) {
    MJLOG_ERR("parse header error");
    return false;
  }
  // stage1: get method type
  const char* method = field->data[0]->data;
  if (!strcmp(method, "GET")) {
    req->method = GET_METHOD;
  } else if (!strcmp(method, "POST")) {
    req->method = POST_METHOD;
  } else if (!strcmp(method, "HEAD")) {
    req->method = HEAD_METHOD;
  } else {
    MJLOG_ERR("method is not support");
    return false;
  }
  // stage2: get access path
  mjstr_copy(req->_path, field->data[1]);
  // stage3: get http version
  const char* version = field->data[2]->data;
  if (!strcmp(version, "HTTP/1.0")) {
    req->_version = "HTTP/1.0";
  } else if (!strcmp(version, "HTTP/1.1")) {
    req->_version = "HTTP/1.1";
  } else {
    MJLOG_ERR("http version error");
    return false;
  }
  // stage4: parse other header
  for (int i = 1; i < header->len; i++) {
    if (header->data[i]) break;
    mjslist_clean(field);
    mjstr_split(header->data[i], ":", field);
    if (field->len < 2) continue;
    mjmap_set_str(req->_header, field->data[0]->data, field->data[1]);
  }
  return true;
}

/*
===============================================================================
mjhttpreq_Delete
  delete mjhttpreq struct
===============================================================================
*/
bool mjhttpreq_delete(mjhttpreq req) {
  if (!req) return false;
  mjstr_delete(req->_path);
	mjstr_delete(req->_query);
  mjmap_delete(req->_header);
	mjslist_delete(req->_htmp);
	mjslist_delete(req->_ftmp);
  free(req);
  return true;
}
