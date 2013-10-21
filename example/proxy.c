#include <stdio.h>
#include <stdlib.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn.h"
#include "mjlog.h"

struct mjproxy {
  mjconn  master;
  mjconn  target;
};
typedef struct mjproxy* mjproxy;

void* mjproxy_clean(void* arg) {
  mjproxy proxy = (mjproxy) arg;
  mjconn_delete(proxy->master);
  mjconn_delete(proxy->target);
  free(proxy);
  return NULL;
}

// just for error check
void* on_write(void* arg) {
  mjconn conn = (mjconn) arg;
  mjproxy proxy = (mjproxy) mjconn_get_obj(conn, "proxy");
  if (conn->_error || conn->_closed || conn->_timeout) {
    MJLOG_ERR("write conn error");
    mjproxy_clean(proxy);
    return NULL;
  }
  return NULL;
}

void* on_masterread(void* arg) {
  mjconn master = (mjconn) arg;
  mjproxy proxy = (mjproxy) mjconn_get_obj(master, "proxy");
  if (master->_error || master->_closed || master->_timeout) {
    MJLOG_ERR("master conn error");
    mjproxy_clean(proxy);
    return NULL;
  }
  mjconn_write(proxy->target, master->_data, on_write);
  mjconn_read(proxy->master, on_masterread);
  return NULL;
}

void* on_targetread(void* arg) {
  mjconn target = (mjconn) arg;
  mjproxy proxy = (mjproxy) mjconn_get_obj(target, "proxy");
  if (target->_error || target->_closed || target->_timeout) {
    MJLOG_ERR("master conn error");
    mjproxy_clean(proxy);
    return NULL;
  }
  mjconn_write(proxy->master, target->_data, on_write);
  mjconn_read(proxy->target, on_targetread);
  return NULL;
}

void* on_targetconnect(void* arg) {
  mjconn target = (mjconn) arg;
  mjproxy proxy = (mjproxy) mjconn_get_obj(target, "proxy");
  if (target->_error || target->_closed || target->_timeout) {
    MJLOG_ERR("connect target error");
    mjproxy_clean(proxy);
    return NULL;
  }
  mjconn_read(proxy->master, on_masterread);
  mjconn_read(proxy->target, on_targetread);
  return NULL;
}

void* on_masterconnect(void *arg) {
  mjconn master = (mjconn) arg;
  // alloc proxy struct and set master conn
  mjproxy proxy = (mjproxy) calloc(1, sizeof(struct mjproxy));
  if (!proxy) {
    MJLOG_ERR("proxy Alloc Error");
    goto failout;
  }
  proxy->master = master;
  mjconn_set_obj(master, "proxy", proxy, NULL);
  // connect to target and set proxy target
  int cfd = mjsock_tcp_socket();
  if (!cfd) {
    MJLOG_ERR("tareget socket create Error");
    goto failout;
  }
  proxy->target = mjconn_new(master->_ev, cfd);
  if (!proxy->target) {
    MJLOG_ERR("tareget create error");
    goto failout;
  }
  mjconn_set_obj(proxy->target, "proxy", proxy, NULL);
  mjconn_connect(proxy->target, "127.0.0.1", 80, on_targetconnect);
  return NULL;

failout:
  mjconn_delete(master);
  return NULL;
}

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (!sfd) {
    printf("socket create error");
    return 1;
  }

  mjtcpsrv srv = mjtcpsrv_new(sfd, on_masterconnect, NULL, NULL, MJTCPSRV_STANDALONE);
  if (!srv) {
    printf("mjtcpsrv create error");
    return 1;
  }
  mjtcpsrv_run(srv);
  mjtcpsrv_delete(srv);
  return 0;
}
