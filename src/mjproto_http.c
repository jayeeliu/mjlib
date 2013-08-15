#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "mjmainsrv.h"
#include "mjtcpsrv.h"
#include "mjlog.h"
#include "mjconn.h"
#include "mjconnb.h"
#include "mjlf.h"
#include "mjproto_http.h"

/*
===============================================================================
on_header
  called in http_Workder
===============================================================================
*/
/*
static void *on_header(void *arg) {
  mjConn conn = (mjConn) arg;
  mjHttpData httpData = (mjHttpData) conn->private;
  mjTcpSrv srv = (mjTcpSrv) conn->server;
  struct mjHttpUrl *urls = (struct mjHttpUrl*) srv->private;
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
  httpData->param = mjstrlist_New();
  if (!httpData->param) {
    MJLOG_ERR("mjstrlist_New error");
    mjConn_Delete(conn);
    return NULL;
  }
  // set default response header
  mjHttpRsp_AddHeader(httpData->response, "Content-Type", 
          "text/html; charset=UTF-8");
  mjHttpRsp_AddHeader(httpData->response, "Server", "SFQ-0.01");
  // call function
  mjstr location = httpData->request->location;
  if (location->data[location->length - 1] != '/') {
    mjstr_CatS(location, "/");
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
*/
/*
===========================================
FileToStr
  read file and store data into mjstr
===========================================
*/
mjstr FileToStr(const char* fileName)
{
  mjstr out = mjstr_new();
  if (!out) {
    MJLOG_ERR("mjstr alloc error");
    return NULL;
  }

  int fd = open(fileName, O_RDONLY);
  if (fd < 0) {
    MJLOG_ERR("open file error");
    mjstr_delete(out);
    return NULL;
  }

  char buf[1024];
  while (1) {
    int ret = read(fd, buf, sizeof(buf));
    if (ret <= 0) break;
    mjstr_catb(out, buf, ret);
  }
  close(fd);

  return out;
}

/*
===============================================================================
http_mjlf_init
  init mjlf according to urls
===============================================================================
*/
void* http_mjlf_init(void* arg) {
  mjthread thread = (mjthread) arg;
  struct mjhttpurl* urls = thread->init_arg;
  // get urls counts
  int count;
  for (count = 0; urls[count].url != NULL; count++);
  // alloc new memory
  struct mjhttpurl* newurls = (struct mjhttpurl*) malloc(
      sizeof(struct mjhttpurl) * (count + 1));
  memcpy(newurls, urls, sizeof(struct mjhttpurl) * (count + 1));
  // init mjreg
  for(int i = 0;  newurls[i].url != NULL; i++) {
    if (!(newurls[i].reg = mjreg_new(newurls[i].url))) {
      MJLOG_ERR("mjreg_new Error");
      return NULL;
    }
  }
  return newurls;
}

/*
===============================================================================
http_mjlf_exit
  free http mapping
===============================================================================
*/
void* http_mjlf_exit(void* arg) {
  mjthread thread = (mjthread) arg;
  struct mjhttpurl* newurls = mjmap_get_obj(thread->arg_map, "thread_local");
  for(int i = 0; newurls[i].url != NULL; i++) {
    mjreg_delete(newurls[i].reg);
  }
  free(newurls);
  return NULL;
}

/*
===============================================================================
create_httpdata
  create new httpdata
===============================================================================
*/
static mjhttpdata create_httpdata(mjstr data) {
  // alloc httpdata
  mjhttpdata httpdata = (mjhttpdata) calloc(1, sizeof(struct mjhttpdata));
  if (!httpdata) {
    MJLOG_ERR("httpdata alloc error");
    return NULL;
  }
  // alloc httpreq object
  httpdata->req = mjhttpreq_new(data);
  if (!httpdata->req) {
    MJLOG_ERR("mjhttpreq_New error");
    free(httpdata);
    return NULL;
  }
  // alloc http params
  httpdata->params = mjstrlist_new();
  if (!httpdata->params) {
    MJLOG_ERR("params alloc error");
    mjhttpreq_delete(httpdata->req);
    free(httpdata);
    return NULL;
  }
  return httpdata;
}
 
/*
===============================================================================
free_httpdata
  free httpdata
===============================================================================
*/
static void* free_httpdata(void* arg) {
  mjhttpdata httpdata = (mjhttpdata) arg;
  mjhttpreq_delete(httpdata->req);
  mjstrlist_delete(httpdata->params);
  free(httpdata);
  return NULL;
}

/*
===============================================================================
http_mjlf_routine
  mjlf_routine run when new conn
===============================================================================
*/
void* http_mjlf_routine(void* arg) {
  mjconnb conn = (mjconnb) arg;
  struct mjhttpurl* urls = (struct mjhttpurl*) conn->shared;
  // alloc data for readuntil
  mjstr data = mjstr_new();
  if (!data) {
    MJLOG_ERR("mjstr alloc error");
    return NULL;
  }
  int ret = mjconnb_readuntil(conn, "\r\n\r\n", data);
  if (ret <= 0) {
    MJLOG_ERR("read http header failed");
    mjstr_delete(data);
    return NULL;
  }
  // alloc httpdata
  mjhttpdata httpdata = create_httpdata(data);
  if (!httpdata) {
    MJLOG_ERR("create httpdata error");
    mjstr_delete(data);
    return NULL;
  }
  // expand location
  mjstr location = httpdata->req->location;
  if (location->data[location->length - 1] != '/') {
    mjstr_cats(location, "/");
  }
  // match proc and run
  int i;
  for (i = 0; urls[i].url != NULL; i++) {
    if (mjreg_search(urls[i].reg, location->data, httpdata->params)) break;
  }
  // set private data
  mjconnb_set_private_data(conn, httpdata, free_httpdata);
  urls[i].fun(conn);
  mjstr_delete(data);
  return NULL;
}
