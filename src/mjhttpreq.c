#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "mjhttpreq.h"
#include "mjlog.h"

#define MAX_HEADER_LEN  20
#define MAX_FIELD_LEN   10

/*
===============================================================================
mjhttpreq_New
  create new mjhttpreq struct
===============================================================================
*/
mjhttpreq mjhttpreq_new(mjstr data) {
  // sanity check
  if (!data) {
    MJLOG_ERR("data is null");
    return NULL;  
  }
  // create mjhttpreq struct
  mjhttpreq request = (mjhttpreq) calloc (1, sizeof(struct mjhttpreq));
  if (!request) {
    MJLOG_ERR("mjhttpreq alloc error");
    return NULL;
  }
  // get all header from data  
  mjstrlist header = mjstrlist_new();
  if (!header) {
    MJLOG_ERR("mjstrlist_New error");
    goto failout1;
  }
  mjstr_split(data, "\r\n", header);
  // get filed from the first len 
  mjstrlist field = mjstrlist_new();
  if (!field) {
    MJLOG_ERR("mjstrlist_New error");
    goto failout2;
  }
  mjstr_split(header->data[0], " ", field);
  // check cmd length
  if (field->length < 2) {
    MJLOG_ERR("parse header error");
    goto failout3;
  }
  // get method type
  const char* method = field->data[0]->data;
  if (!strcasecmp(method, "GET")) { 
    request->method = GET_METHOD;
  } else if (!strcasecmp(method, "POST")) {
    request->method = POST_METHOD;
  } else {
    request->method = UNKNOWN_METHOD;
  }
  // get access location
  request->location = mjstr_new();
  mjstr_copy(request->location, field->data[1]);
  mjstrlist_clean(field);
  // parse other header
  request->req_header = mjmap_new(128);
  for (int i = 1; i < header->length; i++) {
    if (!header->data[i]) break;
    mjstr_split(header->data[i], ":", field);
    if (!field || field->length < 2) continue;
    mjmap_add(request->req_header, field->data[0]->data, field->data[1]);
    mjstrlist_clean(field);
  }
  // clean strlist
  mjstrlist_delete(field);
  mjstrlist_delete(header);
  return request;

failout3:
  mjstrlist_delete(field);
failout2:
  mjstrlist_delete(header);
failout1:
  free(request);
  return NULL;
}

/*
===============================================================================
mjhttpreq_Delete
  delete mjhttpreq struct
===============================================================================
*/
bool mjhttpreq_delete(mjhttpreq request) {
  // sanity check
  if (!request) return false;
  // free struct
  mjstr_delete(request->location);
  mjmap_delete(request->parameter);
  mjmap_delete(request->req_header);
  free(request);
  return true;
}
