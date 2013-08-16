#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mjconn.h"
#include "mjlog.h"
#include "mjsock.h"

#define BUF_SIZE  4096

#define MJCONN_NONE       0
#define MJCONN_READBYTES  1
#define MJCONN_READUNTIL  2
#define MJCONN_READ       3
#define MJCONN_WRITE      10
#define MJCONN_CONN       100

/*
===============================================================================
mjconn_TimeoutCallBack
  read/write timeout callback
===============================================================================
*/
static void* mjconn_timeout_callback(void* data) {
  mjconn conn = (mjconn) data;
  MJLOG_ERR("timeout");
  mjconn_delete(conn);
  return NULL;
}

/*
===============================================================================
mjconn_DelReadEvent
  read finish del event and run callback
===============================================================================
*/
static void* mjconn_del_read_event(mjconn conn) {
  // del read event
  mjev_del_fevent(conn->ev, conn->fd, MJEV_READABLE);
  if (conn->read_timeout) {
    // invalid timer event
    mjev_del_timer(conn->ev, conn->read_timeout_event);
    conn->read_timeout     = 0;
    conn->read_timeout_event  = NULL;
  }
  // reset read type
  conn->read_type = MJCONN_NONE;
  // run callback in the last step
  if (conn->ReadCallBack) conn->ReadCallBack(conn);
  return NULL;
}

/*
===============================================================================
mjconn_ReadEventCallBack
  run when data come
  when error or closed, close conn
===============================================================================
*/
static void* mjconn_read_event_callback(void* arg) {
  mjconn conn = (mjconn) arg;
  char buf[BUF_SIZE];
	// read data in a loop
  for (;;) {
    int ret = read(conn->fd, buf, BUF_SIZE);
    if (ret == -1) {          
      // interrupt by signal continue
      if (errno == EINTR) continue;
      // nonblock, no data, break
      if (errno == EAGAIN) break;
      // error happends
      conn->error = 1;
      MJLOG_ERR("read error");
      break;
    } else if (ret == 0) {
      // read data finish, peer close 
      conn->closed = 1;
      MJLOG_ERR("conn is closed");
      break;
    }
    // read some data, put it to rbuf
    mjstr_catb(conn->rbuf, buf, ret);
    break;
  }
  if (conn->read_type == MJCONN_READBYTES) {
    // readtype is readbytes
    if (conn->rbytes <= conn->rbuf->length) { 
      mjstr_copyb(conn->data, conn->rbuf->data, conn->rbytes);
      mjstr_consume(conn->rbuf, conn->rbytes);
      mjconn_del_read_event(conn);
      return NULL;
    }
  } else if (conn->read_type == MJCONN_READUNTIL) { 
    // read type is readuntil
    int pos = mjstr_search(conn->rbuf, conn->delim);
    if (pos != -1) {
      mjstr_copyb(conn->data, conn->rbuf->data, pos);
      mjstr_consume(conn->rbuf, pos + strlen(conn->delim));
      mjconn_del_read_event(conn);
      return NULL;
    }
  } else if (conn->read_type == MJCONN_READ) { 
    // read type is normal read
    if (conn->rbuf && conn->rbuf->length > 0) {
      mjstr_copyb(conn->data, conn->rbuf->data, conn->rbuf->length);
      mjstr_consume(conn->rbuf, conn->rbuf->length);
      mjconn_del_read_event(conn);
      return NULL;
    }
  }
  // some error happend, close conn
  if (conn->closed || conn->error) {
    mjconn_delete(conn);
  }
  return NULL;
}

/*
===============================================================================
mjconn_add_read_event
  add read event, read timeout event
  return false -- add error, close conn
      true -- add success
===============================================================================
*/
static bool mjconn_add_read_event(mjconn conn) {
  // check if conn has closed or error
  if (conn->closed || conn->error) {
    MJLOG_ERR("conn is closed or error");
    goto failout;
  }
  // add readevent
  if (mjev_add_fevent(conn->ev, conn->fd, MJEV_READABLE, 
    mjconn_read_event_callback, conn) < 0) {
    MJLOG_ERR("mjev_Add error");
    goto failout;
  }
  // add read timeout event
  if (conn->read_timeout) {
    conn->read_timeout_event = mjev_add_timer(conn->ev, conn->read_timeout, 
        mjconn_timeout_callback, conn);
    if (!conn->read_timeout_event) {
      MJLOG_ERR("mjEV_AddTimer error");
      goto failout;
    }
  }
  return true;

failout:
  mjconn_delete(conn);
  return false;
}

/*
================================================================================
mjconn_ReadBytes
  read len bytes
  return false -- error, true -- success
================================================================================
*/
bool mjconn_readbytes(mjconn conn, int len, mjProc CallBack) {
  // sanity check
  if (!conn || !CallBack) {
    MJLOG_ERR("conn or CallBack is null");  
    return false;
  }
  // can't re enter
  if (conn->read_type != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->read_type       = MJCONN_READBYTES;    
  conn->rbytes          = len;
  conn->ReadCallBack    = CallBack;
  // check rbuf
  if (conn->rbytes <= conn->rbuf->length) { 
    // copy rbytes to data
    mjstr_copyb(conn->data, conn->rbuf->data, conn->rbytes);
    mjstr_consume(conn->rbuf, conn->rbytes);
    // read finish
    conn->read_type = MJCONN_NONE;
    // run callback
    if (conn->ReadCallBack) {
      mjev_add_pending(conn->ev, conn->ReadCallBack(conn), conn);
    }
    return true;
  }
  // add to event loop
  return mjconn_add_read_event(conn);
}

/*
===============================================================================
mjconn_ReadUntil
  mjconn read until delim 
  return false --- error, true -- readfinish or set event ok
===============================================================================
*/
bool mjconn_readuntil(mjconn conn, char* delim, mjProc CallBack) {
  // sanity check
  if (!conn || !delim || !CallBack) {
    MJLOG_ERR("conn or delim or proc is null");
    return false;
  }
  // can't re enter
  if (conn->read_type != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->read_type    = MJCONN_READUNTIL;
  conn->delim     = delim;
  conn->ReadCallBack  = CallBack;
  // found data in rbuf, call proc and return 
  int pos = mjstr_search(conn->rbuf, conn->delim);
  if (pos != -1) {
    // copy data to rbuf, not include delim
    mjstr_copyb(conn->data, conn->rbuf->data, pos);
    mjstr_consume(conn->rbuf, pos + strlen(conn->delim));
    // read finish set readType to NONE, run callback
    conn->read_type = MJCONN_NONE;
    // run read callback
    if (conn->ReadCallBack) {
      mjev_add_pending(conn->ev, conn->ReadCallBack(conn), conn);
    }
    return true;
  }
  // add read event to event loop 
  return mjconn_add_read_event(conn); 
}

/*
===============================================================================
mjconn_Read
  read data
===============================================================================
*/
bool mjconn_read(mjconn conn, mjProc CallBack) {
  // sanity check
  if (!conn || !CallBack) {
    MJLOG_ERR("conn or CallBack is null");
    return false;
  }
  // can't re enter
  if (conn->read_type != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->read_type    = MJCONN_READ;
  conn->ReadCallBack  = CallBack;
  // found data in rbuf
  if (conn->rbuf && conn->rbuf->length > 0) {
    mjstr_copyb(conn->data, conn->rbuf->data, conn->rbuf->length);
    mjstr_consume(conn->rbuf, conn->rbuf->length);
    conn->read_type  = MJCONN_NONE;
    if (conn->ReadCallBack) {
      mjev_add_pending(conn->ev, conn->ReadCallBack(conn), conn);
    }
    return 0;
  }
  return mjconn_add_read_event(conn);
}

/*
===============================================================================
mjconn_DelWriteEvent
  del write event
===============================================================================
*/
static void* mjconn_del_write_event(mjconn conn) {
  mjev_del_fevent(conn->ev, conn->fd, MJEV_WRITEABLE);
  // del write timeout event
  if (conn->write_timeout) {
    mjev_del_timer(conn->ev, conn->write_timeout_event);
    conn->write_timeout    = 0;  
    conn->write_timeout_event = NULL;
  }
  // set write type to NONE
  conn->write_type = MJCONN_NONE;
  // call write callback
  if (conn->WriteCallBack) conn->WriteCallBack(conn);
  return NULL;
}

/*
===============================================================================
mjconn_WriteEventCallback
  run when we can write data
===============================================================================
*/
static void* mjconn_write_event_callback(void* arg) {
  mjconn conn = (mjconn)arg;
  int ret = write(conn->fd, conn->wbuf->data, conn->wbuf->length);
  if (ret < 0) {
    MJLOG_ERR("conn write error: %s", strerror(errno));
    mjconn_delete(conn);
    return NULL;
  }
  mjstr_consume(conn->wbuf, ret);
  // no data to write call DelWriteEvent
  if (conn->wbuf->length == 0) {
    mjconn_del_write_event(conn);
  }
  return NULL;
}

/*
===============================================================================
mjconn_AddWriteEvent
  add write event to eventloop
===============================================================================
*/
static bool mjconn_add_write_event(mjconn conn) {
  // sanity check
  if (conn->closed || conn->error) {
    MJLOG_ERR("conn is closed or error");
    goto failout;
  }
  // add write event
  if (mjev_add_fevent(conn->ev, conn->fd, MJEV_WRITEABLE, 
      mjconn_write_event_callback, conn) < 0) {
    MJLOG_ERR("mjEV_Add error");
    goto failout;
  }
  // AddWriteEvent can be call many times
  // When we call it twice, we can't change the callback
  if (conn->write_timeout && !conn->write_timeout_event) { 
    conn->write_timeout_event = mjev_add_timer(conn->ev, 
        conn->write_timeout, mjconn_timeout_callback, conn);
    if (!conn->write_timeout_event) {
      MJLOG_ERR("mjEV_AddTimer error");
      goto failout;
    }
  }
  return true;

failout:
  mjconn_delete(conn);
  return false;
}

/*
===============================================================================
mjconn_BufWriteS
  copy string to wbuf
===============================================================================
*/
bool mjconn_buf_writes(mjconn conn, char* buf) {
  if (!conn || !buf) {
    MJLOG_ERR("conn or buf is null");
    return false;
  }
  mjstr_cats(conn->wbuf, buf);
  return true;
}

/*
===============================================================================
mjconn_BufWrite
  copy mjstr to wbuf
===============================================================================
*/
bool mjconn_buf_write(mjconn conn, mjstr buf) {
  if (!conn || !buf) {
    MJLOG_ERR("conn or buf is null");
    return false;
  }
  return mjconn_buf_writes(conn, buf->data);
}

/*
===============================================================================
mjconn_Flush
  flush wbuf
===============================================================================
*/
bool mjconn_flush(mjconn conn, mjProc CallBack) {
  // sanity check
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  // set write callback
  conn->WriteCallBack = CallBack;
  // if re enter only change callback
  if (conn->write_type == MJCONN_WRITE) return true;
  conn->write_type = MJCONN_WRITE;
  return mjconn_add_write_event(conn);
}

/*
===============================================================================
mjconn_WriteS
  write string
===============================================================================
*/
bool mjconn_writes(mjconn conn, char* buf, mjProc CallBack) {
  if (!conn || !buf) {
    MJLOG_ERR("conn or buf is null");
    return false;
  }
  mjconn_buf_writes(conn, buf);
  return mjconn_flush(conn, CallBack);
}

/*
===============================================================================
mjconn_Write
  write mjstr
===============================================================================
*/
bool mjconn_write(mjconn conn, mjstr buf, mjProc CallBack) {
  if (!conn || !buf) {
    MJLOG_ERR("conn or buf is null");
    return false;
  }
  return mjconn_writes(conn, buf->data, CallBack);
}

/*
===============================================================================
mjconn_SetConnectTimeout
  set conn connect timeout
===============================================================================
*/
bool mjconn_set_connect_timeout(mjconn conn, unsigned int connect_timeout) {
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  conn->connect_timeout = connect_timeout;
  return true;
}

/*
===============================================================================
mjconn_SetTimeout
  set conn, read and write timeout
===============================================================================
*/
bool mjconn_set_timeout(mjconn conn, unsigned int read_timeout, 
      unsigned int write_timeout) {
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  conn->read_timeout   = read_timeout;
  conn->write_timeout  = write_timeout;
  return true;
}

/*
===============================================================================
mjconn_SetPrivate
  set conn private data and private free function
===============================================================================
*/
bool mjconn_set_private(mjconn conn, void* private, mjProc FreePrivte) {
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  conn->private     = private;
  conn->FreePrivte  = FreePrivte;
  return true;
}

/*
===============================================================================
mjconn_SetServer
  set conn server, when conn in server side
===============================================================================
*/
bool mjconn_set_server(mjconn conn, void* server) {
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  conn->server = server;
  return true;
}

/*
===============================================================================
mjconn_DelConnectEvent
  del connect event
===============================================================================
*/
static void* mjconn_del_connect_event(mjconn conn) {
  mjev_del_fevent(conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE); 
  if (conn->connect_timeout) {
    mjev_del_timer(conn->ev, conn->connect_timeout_event);
    conn->connect_timeout    = 0;
    conn->connect_timeout_event   = NULL;
  }
  conn->connect_type = MJCONN_NONE;  
  if (conn->ConnectCallback) conn->ConnectCallback(conn);
  return NULL;
}

/*
===================================================================
mjconn_ConnectEventCallback
  connect callback, successful
===================================================================
*/
static void* mjconn_connect_event_callback(void* arg) {
  mjconn conn = (mjconn)arg;
  int err = 0;
  socklen_t errlen = sizeof(err);
  // get socket status
  if (getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
    MJLOG_ERR("getsockopt error, %s", strerror(errno));
    mjconn_delete(conn);
    return NULL;
  }
  if (err) {
    MJLOG_ERR("err is: %s", strerror(err));
    mjconn_delete(conn);
    return NULL;
  }
  // connect success
  mjconn_del_connect_event(conn); 
  return NULL;
}

/*
===============================================================================
mjconn_AddConnectEvent
  add connect event 
===============================================================================
*/
static bool mjconn_add_connect_event(mjconn conn) {
  // add to eventloop
  if (mjev_add_fevent(conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE, 
        mjconn_connect_event_callback, conn) < 0)  {
    MJLOG_ERR("mjEV_Add error");
    goto failout;
  }
  // set connect timeout
  if (conn->connect_timeout) {
    conn->connect_timeout_event = mjev_add_timer(conn->ev, 
        conn->connect_timeout, mjconn_timeout_callback, conn);
    if (!conn->connect_timeout_event) {
      MJLOG_ERR("mjEV_AddTimer error");
      goto failout;
    }
  }
  return true;

failout:
  mjconn_delete(conn);
  return false;
}

/*
===============================================================================
mjconn_Connect
  connect to host async
===============================================================================
*/
bool mjconn_connect(mjconn conn, const char* ipaddr, int port, 
  mjProc CallBack) {
  // sanity check
  if (!conn || !CallBack) {
    MJLOG_ERR("conn or proc is null");
    return false;
  }
  // can't re enter
  if (conn->connect_type != MJCONN_NONE) {
    MJLOG_ERR("connectType must be MJCONN_NONE");
    return false;
  }
  // set conn type and callback
  conn->connect_type    = MJCONN_CONN;
  conn->ConnectCallback  = CallBack;
  // init address
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  inet_pton(AF_INET, ipaddr, &addr.sin_addr);
  // try to connect 
  int ret = connect(conn->fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == 0) {       
    // connect success
    conn->connect_type = MJCONN_NONE;
    if (conn->ConnectCallback) {
      mjev_add_pending(conn->ev, conn->ConnectCallback, conn);
    }
    return true;
  }
  // connect failed, set nonblock connect
  if (errno == EINPROGRESS) return mjconn_add_connect_event(conn);
  MJLOG_ERR("connect failed");  
  mjconn_delete(conn);
  return false;
}

// conn buffer
#define MAX_FD    60000
static struct mjconn _conn[MAX_FD];

/*
===============================================================================
mjconn_SetBuffer
  used by mjconn_New for init buffer
===============================================================================
*/
static mjstr mjconn_set_buffer(mjstr defVal) {
  if (defVal) return defVal;
  return mjstr_new();
}

/*
===============================================================================
mjconn_New
  create mjconn
  return NULL -- fail, other -- success
===============================================================================
*/
mjconn mjconn_new(mjev ev, int fd) {
  // event loop must not be null
  if (!ev) {
    MJLOG_ERR("ev is null");
    return NULL;   
  }
  if (fd > MAX_FD) {
    MJLOG_ERR("fd is too large");
    return NULL;
  }
  // set fd to nonblock 
  mjsock_set_blocking(fd, 0);
  // alloc mjconn struct 
  mjconn conn = &_conn[fd];
  mjstr rbak = conn->rbuf;
  mjstr wbak = conn->wbuf;
  mjstr dbak = conn->data;
  // clean mjconn
  memset(conn, 0, sizeof(struct mjconn));
  conn->fd = fd;       // set conn fd 
  conn->ev = ev;       // set ev
  // create buffer
  conn->rbuf = mjconn_set_buffer(rbak);
  conn->wbuf = mjconn_set_buffer(wbak);
  conn->data = mjconn_set_buffer(dbak);
  if (!conn->rbuf || !conn->wbuf || !conn->data) {
    MJLOG_ERR("mjstr create error");
    mjsock_close(fd);
    return NULL;
  }
  return conn;
}

/*
===============================================================================
mjconn_Delete
  delete mjconn struct
===============================================================================
*/
bool mjconn_delete(mjconn conn) {
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  // invalid connect timeout event
  if (conn->connect_timeout) {
    mjev_del_timer(conn->ev, conn->connect_timeout_event);
    conn->connect_timeout    = 0;
    conn->connect_timeout_event   = NULL;
  }
  // invalid read timeout event
  if (conn->read_timeout) {
    mjev_del_timer(conn->ev, conn->read_timeout_event); 
    conn->read_timeout     = 0;
    conn->read_timeout_event  = NULL;
  }
  // invalid write timeout event
  if (conn->write_timeout) {
    mjev_del_timer(conn->ev, conn->write_timeout_event);
    conn->write_timeout    = 0;
    conn->write_timeout_event = NULL;
  }
  // free private data
  if (conn->private && conn->FreePrivte) { 
    conn->FreePrivte(conn->private);
  }
  // delete eventloop fd, pending proc
  mjev_del_fevent(conn->ev, conn->fd, MJEV_READABLE | MJEV_WRITEABLE);
  mjev_del_pending(conn->ev, conn);
  mjsock_close(conn->fd);
  return true;
}
