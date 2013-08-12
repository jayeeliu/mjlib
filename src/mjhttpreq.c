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
mjhttpreq mjhttpreq_new(mjStr data) {
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
  mjStrList header = mjStrList_New();
  if (!header) {
    MJLOG_ERR("mjStrList_New error");
    goto failout1;
  }
  mjStr_Split(data, "\r\n", header);
  // get filed from the first len 
  mjStrList field = mjStrList_New();
  if (!field) {
    MJLOG_ERR("mjStrList_New error");
    goto failout2;
  }
  mjStr_Split(header->data[0], " ", field);
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
  request->location = mjStr_New();
  mjStr_Copy(request->location, field->data[1]);
  mjStrList_Clean(field);
  // parse other header
  request->req_header = mjMap_New(128);
  for (int i = 1; i < header->length; i++) {
    if (!header->data[i]) break;
    mjStr_Split(header->data[i], ":", field);
    if (!field || field->length < 2) continue;
    mjMap_Add(request->req_header, field->data[0]->data, field->data[1]);
    mjStrList_Clean(field);
  }
  // clean strlist
  mjStrList_Delete(field);
  mjStrList_Delete(header);
  return request;

failout3:
  mjStrList_Delete(field);
failout2:
  mjStrList_Delete(header);
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
  mjStr_Delete(request->location);
  mjMap_Delete(request->parameter);
  mjMap_Delete(request->req_header);
  free(request);
  return true;
}
