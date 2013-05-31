#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "mjmainsrv.h"
#include "mjtcpsrv.h"
#include "mjlog.h"
#include "mjconn.h"
#include "mjproto_http.h"

/*
===============================================================================
on_header
  called in http_Workder
===============================================================================
*/
static void *on_header(void *arg) {
  mjConn conn = (mjConn) arg;
  mjHttpData httpData = (mjHttpData) conn->private;
  mjTcpSrv srv = (mjTcpSrv) conn->server;
  mjMainSrv mainSrv = srv->mainSrv;
  struct mjHttpUrl *urls = NULL;
  // check main server or tcpserver
  if (!mainSrv) {
    urls = (struct mjHttpUrl*) srv->private;
  } else {
    urls = (struct mjHttpUrl*) mainSrv->private; 
  }
  // alloc request and response struct 
  httpData->request = mjHttpReq_New(conn->data);
  if (!httpData->request) {
    MJLOG_ERR("mjHttpReq_New error");
    mjConn_Delete(conn);
    return NULL;
  }
  httpData->response = mjHttpRsp_New();
  if (!httpData->response) {
    MJLOG_ERR("mjHttpRsp_new error");
    mjConn_Delete(conn);
    return NULL;
  }
  httpData->param = mjStrList_New();
  if (!httpData->param) {
    MJLOG_ERR("mjStrList_New error");
    mjConn_Delete(conn);
    return NULL;
  }
  // set default response header
  mjHttpRsp_AddHeader(httpData->response, "Content-Type", 
          "text/html; charset=UTF-8");
  mjHttpRsp_AddHeader(httpData->response, "Server", "SFQ-0.01");
  // call function
  mjStr location = httpData->request->location;
  if (location->data[location->length - 1] != '/') {
    mjStr_CatS(location, "/");
  }
  // check string match
  int i;
  for (i = 0; urls[i].url != NULL; i++) {
    if (mjReg_Search(urls[i].reg, location->data, httpData->param)) {
      urls[i].fun(conn);
      return NULL;
    }
  }
  // run default function
  urls[i].fun(conn);
  return NULL;
}

/*
===============================================================================
httpData_free
  free per conn data
===============================================================================
*/
static void* httpData_free(void *arg) {
  mjHttpData httpData = (mjHttpData) arg;
  if (httpData->request) mjHttpReq_Delete(httpData->request);
  if (httpData->response) mjHttpRsp_Delete(httpData->response);
  if (httpData->param) mjStrList_Delete(httpData->param);
  free(httpData);
  return NULL;
}

/*
===============================================================================
http_worker
  main worker of http protocol
  alloc per conn data, store in private
  read header
===============================================================================
*/
void* http_Worker(void* arg) {
  mjConn conn = (mjConn) arg;
  void *httpData = calloc(1, sizeof(struct mjHttpData));
  if (!httpData) {
    MJLOG_ERR("httpData alloc error");
    mjConn_Delete(conn);
    return NULL;
  }
  // set conn private data
  mjConn_SetPrivate(conn, httpData, httpData_free);
  mjConn_ReadUntil(conn, "\r\n\r\n", on_header); 
  return NULL;
}

/*
===============================================================================
http_InitSrv
  called when httpserver run
  urls must be set first
  create mjreg
===============================================================================
*/
static void* http_InitSrv(struct mjHttpUrl *urls) {
  if (!urls) {
    MJLOG_ERR("Oops urls is null");
    return NULL;
  }
  // create mjreg
  for (int i = 0; urls[i].url != NULL; i++) {
    urls[i].reg = mjReg_New(urls[i].url);
    if (!urls[i].reg) {
      MJLOG_ERR("mjreg_new Error");
      return NULL;
    }
  }
  return NULL;
}

void* http_InitTcpSrv(void *arg) {
  mjTcpSrv srv = (mjTcpSrv) arg;
  return http_InitSrv((struct mjHttpUrl*) srv->private);
}

void* http_InitMainSrv(void *arg) {
  mjMainSrv mainSrv = (mjMainSrv) arg;
  return http_InitSrv((struct mjHttpUrl*) mainSrv->private);
}

/*
===============================================================================
http_ExitSrv
  called when httpserver exit
  delete mjreg
===============================================================================
*/
static void* http_ExitSrv(struct mjHttpUrl *urls) {
  if (!urls) {
    MJLOG_ERR("Oops urls is null");
    return NULL;
  }
  // delete mjReg
  for (int i = 0; urls[i].url != NULL; i++) {
    if (!urls[i].reg) mjReg_Delete(urls[i].reg);
  }
  return NULL;
}

void* http_ExitTcpSrv(void *arg) {
  mjTcpSrv srv = (mjTcpSrv) arg;
  return http_ExitSrv(srv->private);
}

void* http_ExitMainSrv(void* arg) {
  mjMainSrv mainSrv = (mjMainSrv) arg;
  return http_ExitSrv(mainSrv->private);
}

/*
===========================================
FileToStr
  read file and store data into mjStr
===========================================
*/
mjStr FileToStr(const char* fileName)
{
  mjStr out = mjStr_New();
  if (!out) {
    MJLOG_ERR("mjStr alloc error");
    return NULL;
  }

  int fd = open(fileName, O_RDONLY);
  if (fd < 0) {
    MJLOG_ERR("open file error");
    mjStr_Delete(out);
    return NULL;
  }

  char buf[1024];
  while (1) {
    int ret = read(fd, buf, sizeof(buf));
    if (ret <= 0) break;
    mjStr_CatB(out, buf, ret);
  }
  close(fd);

  return out;
}
