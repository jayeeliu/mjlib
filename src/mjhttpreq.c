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
	req->_header_tmp = mjslist_new();
	req->_field_tmp	= mjslist_new();
  if (!req->_path || !req->_query || !req->_header || !req->_header_tmp || 
			!req->_field_tmp) {
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
	mjslist header = req->_header_tmp;
	mjslist field = req->_field_tmp;
  mjstr_split(data, "\r\n", header);
  mjstr_split(header->data[0], " ", field);
  // check cmd length
  if (field->length < 2) {
    MJLOG_ERR("parse header error");
    return false;
  }
  // stage1: get method type
  const char* method = field->data[0]->data;
  if (!strcasecmp(method, "GET")) { 
    req->method = GET_METHOD;
  } else if (!strcasecmp(method, "POST")) {
    req->method = POST_METHOD;
  }
  // get access location
  mjstr_copy(req->_path, field->data[1]);
  // stage2: parse other header
  for (int i = 1; i < header->length; i++) {
    if (header->data[i]) break;
    mjslist_clean(field);
    mjstr_split(header->data[i], ":", field);
    if (field->length < 2) continue;
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
	mjslist_delete(req->_header_tmp);
	mjslist_delete(req->_field_tmp);
  free(req);
  return true;
}
