#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "mjsrv.h"
#include "mjlog.h"
#include "mjconn.h"
#include "mjconnb.h"
#include "mjlf.h"
#include "mjproto_http.h"

/*
===============================================================================
FileToStr
  read file and store data into mjstr
===============================================================================
*/
mjstr file_to_str(const char* filename) {
  // open file
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    MJLOG_ERR("open file error");
    return NULL;
  }
  // creat out string
  mjstr out = mjstr_new(80);
  if (!out) {
    MJLOG_ERR("mjstr alloc error");
    return NULL;
  }
  // read file 
  char buf[1024];
  while (1) {
    int ret = read(fd, buf, sizeof(buf));
    if (ret <= 0) break;
    mjstr_catb(out, buf, ret);
  }
  close(fd);
  return out;
}

static void* http_free_urls(void* arg);
/*
===============================================================================
http_init_urls
  init urls according to old urls
===============================================================================
*/
static struct mjhttpurl* http_init_urls(struct mjhttpurl* oldurls) {
  // get old urls count
  int count;
  for (count = 0; oldurls[count].url != NULL; count++);
  // alloc new memory, and copy
  struct mjhttpurl* urls = (struct mjhttpurl*) malloc(
      sizeof(struct mjhttpurl) * (count + 1));
  memcpy(urls, oldurls, sizeof(struct mjhttpurl) * (count + 1));
  // init mjreg
  for(int i = 0; urls[i].url != NULL; i++) {
    if (!(urls[i].reg = mjreg_new(urls[i].url))) {
      MJLOG_ERR("mjreg_new Error");
      http_free_urls(urls);
      return NULL;
    }
  }
  return urls;
}

/*
===============================================================================
http_free_urls
  free http urls
===============================================================================
*/
static void* http_free_urls(void* arg) {
  struct mjhttpurl* urls = arg;
  for(int i = 0; urls[i].url != NULL; i++) mjreg_delete(urls[i].reg);
  free(urls);
  return NULL;
}

/*
===============================================================================
http_mjlf_init
  init mjlf according to urls
===============================================================================
*/
void* http_mjlf_init(mjlf srv, void* arg) {
  mjlf_set_local(srv, "urls", http_init_urls(arg), http_free_urls);
  return NULL;
}

/*
===============================================================================
http_mjsrv_init
  init mjsrv according to urls
===============================================================================
*/
void* http_mjsrv_init(mjsrv srv, void* arg) {
  mjsrv_set_local(srv, "urls", http_init_urls(arg), http_free_urls);
  return NULL;
}

static void* mjhttpdata_delete(void* arg);
/*
===============================================================================
create_httpdata
  create new httpdata
===============================================================================
*/
static mjhttpdata mjhttpdata_new(mjstr data) {
  // alloc httpdata
  mjhttpdata hdata = (mjhttpdata) calloc(1, sizeof(struct mjhttpdata));
  if (!hdata) {
    MJLOG_ERR("httpdata alloc error");
    return NULL;
  }
  // alloc httpdata object
  hdata->req = mjhttpreq_new();
  hdata->rsp = mjhttprsp_new();
  hdata->params = mjslist_new();
  if (!hdata->req || !hdata->rsp || !hdata->params) {
    MJLOG_ERR("httpdata alloc error");
    mjhttpdata_delete(hdata);
    return NULL;
  }
  // parse request header
  if (!mjhttpreq_parse(hdata->req, data)) {
    MJLOG_ERR("mjhttpreq_parse error");
    mjhttpdata_delete(hdata);
    return NULL;
  }
  return hdata;
}
 
/*
===============================================================================
free_httpdata
  free httpdata
===============================================================================
*/
static void* mjhttpdata_delete(void* arg) {
  mjhttpdata hdata = (mjhttpdata) arg;
  mjhttpreq_delete(hdata->req);
  mjhttprsp_delete(hdata->rsp);
  mjslist_delete(hdata->params);
  free(hdata);
  return NULL;
}

static mjProc find_url_func(mjhttpdata hdata, struct mjhttpurl* urls) {
  // expand path
  mjstr path = hdata->req->_path;
  if (path->data[path->len-1] != '/') mjstr_cats(path, "/");
  // find func
  int i;
  for (i = 0; urls[i].url != NULL; i++) {
    if (mjreg_search(urls[i].reg, path->data, hdata->params)) break;
  }
  return urls[i].fun;
}

/*
===============================================================================
http_mjlf_routine
  mjlf_routine run when new conn
===============================================================================
*/
void* http_mjlf_routine(void* arg) {
  mjconnb conn = (mjconnb) arg;
  // alloc data for readuntil
  mjstr data = mjstr_new(128);
  if (!data) {
    MJLOG_ERR("mjstr alloc error");
    return NULL;
  }
  int ret = mjconnb_readuntil(conn, "\r\n\r\n", data);
  if (ret <= 0) {
    MJLOG_ERR("read http header failed");
    goto out;
  }
  // alloc httpdata
  mjhttpdata hdata = mjhttpdata_new(data);
  if (!hdata) {
    MJLOG_ERR("create httpdata error");
    goto out;
  }
  // match proc and run
  mjlf srv = (mjlf) mjconnb_get_obj(conn, "server");
  struct mjhttpurl* urls = (struct mjhttpurl*) mjlf_get_local(srv, "urls");
  mjProc fun = find_url_func(hdata, urls);
  mjconnb_set_obj(conn, "httpdata", hdata, mjhttpdata_delete);
  fun(conn);
out:
  mjstr_delete(data);
  return NULL;
}

/*
===============================================================================
on_header
  called in http_mjsrv_routine
===============================================================================
*/
static void *on_header(void *arg) {
  mjconn conn = (mjconn) arg;
  // conn error
  if (conn->_error || conn->_closed || conn->_timeout) {
    mjconn_delete(conn);
    return NULL;
  }
  // create httpdata
  mjhttpdata hdata = mjhttpdata_new(conn->_data);
  if (!hdata) {
    MJLOG_ERR("create hdata error");
    mjconn_delete(conn);
    return NULL;
  }
  mjhttprsp_add_header(hdata->rsp, "Content-Type", "text/html; charset=UTF-8");
  // match proc and run
  mjsrv srv = (mjsrv) mjconn_get_obj(conn, "server");
  struct mjhttpurl* urls = (struct mjhttpurl*) mjsrv_get_local(srv, "urls");
  mjProc fun = find_url_func(hdata, urls);
  mjconn_set_obj(conn, "httpdata", hdata, mjhttpdata_delete);
  fun(conn);
  return NULL;
}

void* http_mjsrv_routine(void* arg) {
  mjconn conn = (mjconn) arg;
  mjconn_readuntil(conn, "\r\n\r\n", on_header);
  return NULL;
}
