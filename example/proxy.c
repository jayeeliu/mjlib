#include <stdio.h>
#include <stdlib.h>
#include "mjtcpsrv.h"
#include "mjsock.h"
#include "mjopt.h"
#include "mjconn.h"
#include "mjlog.h"

struct Proxy {
  mjConn  master;
  mjConn  target;
  int     connFlag;
};
typedef struct Proxy* Proxy;

void* On_Close(void* arg) {
  mjConn conn = (mjConn) arg;
  mjConn_Delete(conn);
  return NULL;
}

void* On_CleanMaster(void* arg) {
  Proxy proxy = (Proxy) arg;
  if (proxy->target) mjConn_Delete(proxy->target);
  free(proxy);
  return NULL;
}

/*
===============================================================================
On_CleanTarget
  close Target conn
===============================================================================
*/
void* On_CleanTarget(void* arg) {
  Proxy proxy = (Proxy) arg;
  proxy->target = NULL;
  proxy->connFlag = 0;
  return NULL;
}

void* On_TargetConnect(void* arg) {
  mjConn target = (mjConn) arg;
  Proxy proxy = (Proxy) target->private;
  proxy->connFlag = 1;
  return NULL;
}

void* On_Read(void* arg) {
  mjConn master = (mjConn) arg;
  Proxy proxy = (Proxy) master->private;
  if (proxy->target) {
    mjConn_WriteS(master, "Connect to target OK\r\n", On_Close);
  } else {
    mjConn_WriteS(master, "Connect to target FAILD\r\n", On_Close);
  }
  return NULL;
}

void* On_Connect(void *arg) {
  mjConn master = (mjConn) arg;
  // alloc proxy struct and set master conn
  Proxy proxy = (Proxy) calloc(1, sizeof(struct Proxy));
  if (!proxy) {
    MJLOG_ERR("Proxy Alloc Error");
    goto failout;
  }
  proxy->master = master;
  mjConn_SetPrivate(master, proxy, On_CleanMaster);
  mjConn_ReadUntil(master, "\r\n\r\n", On_Read);
  // connect to target
  int cfd = mjSock_TcpSocket();
  if (!cfd) {
    MJLOG_ERR("tareget socket create Error");
    goto failout;
  }
  proxy->target = mjConn_New(master->ev, cfd);
  if (!proxy->target) {
    MJLOG_ERR("tareget create error");
    goto failout;
  }
  mjConn_SetPrivate(proxy->target, proxy, On_CleanTarget);
  mjConn_Connect(proxy->target, "127.0.0.1", 80, On_TargetConnect);
  return NULL;
failout:
  mjConn_Delete(master);
  return NULL;
}

int main() {
  int sfd = mjSock_TcpServer(7879);
  if (!sfd) {
    printf("socket create error");
    return 1;
  }

  mjtcpsrv srv = mjtcpsrv_new(sfd, On_Connect, MJTCPSRV_STANDALONE);
  if (!srv) {
    printf("mjTcpSrv create error");
    return 1;
  }
  mjtcpsrv_run(srv);
  mjtcpsrv_delete(srv);
  return 0;
}
