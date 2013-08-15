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
  req->location = mjstr_new();
  if (!req->location) {
    MJLOG_ERR("mjstr_new error");
    free(req);
    return NULL;
  }
  // alloc req_header 
  req->req_header = mjmap_new(131);
  if (!req->req_header) {
    MJLOG_ERR("mjmap_new error");
    mjstr_delete(req->location);
    free(req);
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
bool mjhttpreq_init(mjhttpreq req, mjstr data) {
  // sanity check
  if (!data) {
    MJLOG_ERR("data is null");
    return false;  
  }
  // get all header from data  
  mjstrlist header = mjstrlist_new();
  if (!header) {
    MJLOG_ERR("mjstrlist_New error");
    return false;
  }
  mjstr_split(data, "\r\n", header);
  // get filed from the first len 
  mjstrlist field = mjstrlist_new();
  if (!field) {
    MJLOG_ERR("mjstrlist_New error");
    goto failout1;
  }
  mjstr_split(header->data[0], " ", field);
  // check cmd length
  if (field->length < 2) {
    MJLOG_ERR("parse header error");
    goto failout2;
  }
  // get method type
  const char* method = field->data[0]->data;
  if (!strcasecmp(method, "GET")) { 
    req->method = GET_METHOD;
  } else if (!strcasecmp(method, "POST")) {
    req->method = POST_METHOD;
  } else {
    req->method = UNKNOWN_METHOD;
  }
  // get access location
  mjstr_copy(req->location, field->data[1]);
  mjstrlist_clean(field);
  // parse other header
  for (int i = 1; i < header->length; i++) {
    if (!header->data[i]) break;
    mjstr_split(header->data[i], ":", field);
    if (!field || field->length < 2) continue;
    mjmap_set_str(req->req_header, field->data[0]->data, field->data[1]);
    mjstrlist_clean(field);
  }
  // clean strlist
  mjstrlist_delete(field);
  mjstrlist_delete(header);
  return true;

failout2:
  mjstrlist_delete(field);
failout1:
  mjstrlist_delete(header);
  return false;
}

/*
===============================================================================
mjhttpreq_Delete
  delete mjhttpreq struct
===============================================================================
*/
bool mjhttpreq_delete(mjhttpreq req) {
  // sanity check
  if (!req) return false;
  // free struct
  mjstr_delete(req->location);
  mjmap_delete(req->req_header);
  free(req);
  return true;
}
