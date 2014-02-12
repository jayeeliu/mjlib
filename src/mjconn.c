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
mjconn_del_revent
  read finish del event and run callback
===============================================================================
*/
static void* mjconn_del_revent(mjconn conn) {
  // del read event, invalid timer event
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE);
  if (conn->_rto_e) {
    mjev_del_timer(conn->_ev, conn->_rto_e);
    conn->_rto_e = NULL;
  }
  conn->_rtype = MJCONN_NONE;
  // run callback in the last step
  if (conn->_RCB) conn->_RCB(conn);
  return NULL;
}

/*
===============================================================================
mjconn_ReadEventCallBack
  run when data come
  when error or closed, close conn
===============================================================================
*/
static void* mjconn_revent_cb(void* arg) {
  mjconn conn = (mjconn) arg;
  char buf[BUF_SIZE];
	// read data in a loop
  for (;;) {
    int ret = read(conn->_fd, buf, BUF_SIZE);
    if (ret == -1) {          
      if (errno == EINTR) continue;
      if (errno == EAGAIN) break;
      // error happends
      conn->_error = true;
      MJLOG_ERR("read error");
      break;
    } else if (ret == 0) {
      // read data finish, peer close 
      conn->_closed = true;
      MJLOG_ERR("conn is closed");
      break;
    }
    // read some data, put it to rbuf
    mjstr_catb(conn->_rbuf, buf, ret);
    break;
  }
  if (conn->_rtype == MJCONN_READBYTES) {
    // readtype is readbytes
    if (conn->_rbytes <= conn->_rbuf->len) { 
      mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbytes);
      mjstr_consume(conn->_rbuf, conn->_rbytes);
      mjconn_del_revent(conn);
      return NULL;
    }
  } else if (conn->_rtype == MJCONN_READUNTIL) { 
    // read type is readuntil
    int pos = mjstr_search(conn->_rbuf, conn->_delim);
    if (pos != -1) {
      mjstr_copyb(conn->_data, conn->_rbuf->data, pos);
      mjstr_consume(conn->_rbuf, pos + strlen(conn->_delim));
      mjconn_del_revent(conn);
      return NULL;
    }
  } else if (conn->_rtype == MJCONN_READ) { 
    // read type is normal read
    if (conn->_rbuf && conn->_rbuf->len > 0) {
      mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbuf->len);
      mjstr_consume(conn->_rbuf, conn->_rbuf->len);
      mjconn_del_revent(conn);
      return NULL;
    }
  }
  // some error happend, close conn
  if (conn->_closed || conn->_error) mjconn_del_revent(conn);
  return NULL;
}

/*
===============================================================================
mjconn_TimeoutCallBack
  read/write timeout callback
===============================================================================
*/
static void* mjconn_rto_cb(void* arg) {
  mjconn conn = (mjconn) arg;
  conn->_timeout = true;
  mjconn_del_revent(conn);
  return NULL;
}

/*
===============================================================================
mjconn_add_revent
  add read event, read timeout event
  return false -- add error, close conn
      true -- add success
===============================================================================
*/
static bool mjconn_add_revent(mjconn conn) {
  if (mjev_add_fevent(conn->_ev, conn->_fd, MJEV_READABLE, 
        mjconn_revent_cb, conn) < 0) {
    MJLOG_ERR("mjev_add error");
    goto failout;
  }
  // add read timeout event
  if (conn->_rto) {
    if (conn->_rto_e) {
      MJLOG_ERR("mjconn read time event not null");
      mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE);
      goto failout;
    }
    conn->_rto_e = mjev_add_timer(conn->_ev, conn->_rto, mjconn_rto_cb, conn);
    if (!conn->_rto_e) {
      MJLOG_ERR("mjev_addtimer error");
      mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE);
      goto failout;
    }
  }
  return true;

failout:
  conn->_error = true;
  return false;
}

/*
================================================================================
mjconn_readbytes
  read len bytes, if finished call CB(connect routine)
  return false -- error, true -- success
================================================================================
*/
bool mjconn_readbytes(mjconn conn, int len, mjProc CB) {
  // sanity check
  if (!conn || !CB || conn->_error || conn->_closed || conn->_timeout) {
    MJLOG_ERR("conn error");  
    return false;
  }
  // can't re enter
  if (conn->_rtype != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->_rtype  = MJCONN_READBYTES;    
  conn->_rbytes = len;
  conn->_RCB    = CB;
  // check rbuf
  if (conn->_rbytes <= conn->_rbuf->len) { 
    // read finish, copy rbytes to data
    mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbytes);
    mjstr_consume(conn->_rbuf, conn->_rbytes);
    conn->_rtype = MJCONN_NONE;
    // run callback in pending
    if (conn->_RCB) mjev_add_pending(conn->_ev, conn->_RCB, conn);
    return true;
  }
  // add to event loop
  return mjconn_add_revent(conn);
}

/*
===============================================================================
mjconn_ReadUntil
  mjconn read until delim 
  return false --- error, true -- readfinish or set event ok
===============================================================================
*/
bool mjconn_readuntil(mjconn conn, char* delim, mjProc CB) {
  // sanity check
  if (!conn || !delim || conn->_error || conn->_closed || conn->_timeout 
      || !CB) {
    MJLOG_ERR("conn or delim or proc is null");
    return false;
  }
  // can't re enter
  if (conn->_rtype != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->_rtype  = MJCONN_READUNTIL;
  conn->_delim  = delim;
  conn->_RCB	  = CB;
  // check rbuf
  int pos = mjstr_search(conn->_rbuf, conn->_delim);
  if (pos != -1) {
    // copy data to rbuf, not include delim
    mjstr_copyb(conn->_data, conn->_rbuf->data, pos);
    mjstr_consume(conn->_rbuf, pos + strlen(conn->_delim));
    conn->_rtype = MJCONN_NONE;
    // run callback in pending
    if (conn->_RCB) mjev_add_pending(conn->_ev, conn->_RCB, conn);
    return true;
  }
  // add read event to event loop 
  return mjconn_add_revent(conn); 
}

/*
===============================================================================
mjconn_Read
  read data
===============================================================================
*/
bool mjconn_read(mjconn conn, mjProc CB) {
  // sanity check
  if (!conn || !CB || conn->_error || conn->_closed || conn->_timeout) {
    MJLOG_ERR("conn error");
    return false;
  }
  // can't re enter
  if (conn->_rtype != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->_rtype  = MJCONN_READ;
  conn->_RCB	  = CB;
  // check rbuf
  if (conn->_rbuf && conn->_rbuf->len > 0) {
    mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbuf->len);
    mjstr_consume(conn->_rbuf, conn->_rbuf->len);
    conn->_rtype = MJCONN_NONE;
    // run callback in pending
    if (conn->_RCB) mjev_add_pending(conn->_ev, conn->_RCB, conn);
    return true;
  }
  return mjconn_add_revent(conn);
}

/*
===============================================================================
mjconn_DelWriteEvent
  del write event
===============================================================================
*/
static void* mjconn_del_wevent(mjconn conn) {
  // del write event, invalid timer event
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_WRITEABLE);
  if (conn->_wto_e) {
    mjev_del_timer(conn->_ev, conn->_wto_e);
    conn->_wto_e = NULL;
  }
  conn->_wtype = MJCONN_NONE;
  // call write callback
  if (conn->_WCB) conn->_WCB(conn);
  return NULL;
}

/*
===============================================================================
mjconn_wevent_cb
  run when we can write data
===============================================================================
*/
static void* mjconn_wevent_cb(void* arg) {
  mjconn conn = (mjconn)arg;
  int ret = write(conn->_fd, conn->_wbuf->data, conn->_wbuf->len);
  if (ret < 0) {
    MJLOG_ERR("conn write error: %s", strerror(errno));
    conn->_error = true;
    mjconn_del_wevent(conn);
    return NULL;
  }
  mjstr_consume(conn->_wbuf, ret);
  // no data to write call mjconn_del_wevent
  if (conn->_wbuf->len == 0) mjconn_del_wevent(conn);
  return NULL;
}

/*
===============================================================================
mjconn_TimeoutCallBack
  read/write timeout callback
===============================================================================
*/
static void* mjconn_wto_cb(void* arg) {
  mjconn conn = arg;
  conn->_timeout = true;
  mjconn_del_wevent(conn);
  return NULL;
}

/*
===============================================================================
mjconn_add_wevent
  add write event to eventloop
===============================================================================
*/
static bool mjconn_add_wevent(mjconn conn) {
  // add write event
  if (mjev_add_fevent(conn->_ev, conn->_fd, MJEV_WRITEABLE, mjconn_wevent_cb, 
      conn) < 0) {
    MJLOG_ERR("mjev_add error");
    goto failout;
  }
  // AddWriteEvent can be call many times
  // When we call it twice, we can't change the callback
  if (conn->_wto) { 
    if (conn->_wto_e) {
      MJLOG_ERR("mjconn write time event not null");
      mjev_del_fevent(conn->_ev, conn->_fd, MJEV_WRITEABLE);
      goto failout;
    }
    conn->_wto_e = mjev_add_timer(conn->_ev, conn->_wto, mjconn_wto_cb, conn);
    if (!conn->_wto_e) {
      MJLOG_ERR("mjev_addtimer error");
      mjev_del_fevent(conn->_ev, conn->_fd, MJEV_WRITEABLE);
      goto failout;
    }
  }
  return true;

failout:
  conn->_error = true;
  return false;
}

/*
===============================================================================
mjconn_buf_writeb
  write data to conn
===============================================================================
*/
bool mjconn_buf_writeb(mjconn conn, char* buf, int length) {
  if (!conn || !buf || conn->_error || conn->_closed) {
    MJLOG_ERR("conn error");
    return false;
  }
  mjstr_catb(conn->_wbuf, buf, length);
  return true;
}

/*
===============================================================================
mjconn_Flush
  flush wbuf
===============================================================================
*/
bool mjconn_flush(mjconn conn, mjProc CB) {
  // sanity check
  if (!conn || conn->_error || conn->_closed) {
    MJLOG_ERR("conn error");
    return false;
  }
  if (conn->_wtype != MJCONN_NONE) {
    MJLOG_ERR("conn write type should be null");
    return false;
  }
  // set write callback
  conn->_WCB    = CB;
  conn->_wtype  = MJCONN_WRITE;
  return mjconn_add_wevent(conn);
}

/*
===============================================================================
mjconn_writeb
  write data to conn
===============================================================================
*/
bool mjconn_writeb(mjconn conn, char* buf, int length, mjProc CB) {
  if (!conn || !buf || conn->_error || conn->_closed) {
    MJLOG_ERR("conn error");
    return false;
  }
  if (conn->_wtype != MJCONN_NONE) {
    MJLOG_ERR("conn write type should be null");
    return false;
  }
  mjconn_buf_writeb(conn, buf, length);
  return mjconn_flush(conn, CB);
}

/*
===============================================================================
mjconn_DelConnectEvent
  del connect event
===============================================================================
*/
static void* mjconn_del_cevent(mjconn conn) {
  // delete conn event, invalid timer
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE); 
  if (conn->_cto_e) {
    mjev_del_timer(conn->_ev, conn->_cto_e);
    conn->_cto_e = NULL;
  }
  conn->_ctype = MJCONN_NONE;  
  if (conn->_CCB) conn->_CCB(conn);
  return NULL;
}

/*
===================================================================
mjconn_ConnectEventCallback
  connect callback, successful
===================================================================
*/
static void* mjconn_cevent_cb(void* arg) {
  mjconn conn = (mjconn)arg;
  int err = 0;
  socklen_t errlen = sizeof(err);
  // get socket status
  if (getsockopt(conn->_fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
    MJLOG_ERR("getsockopt error, %s", strerror(errno));
    conn->_error = true;
    mjconn_del_cevent(conn); 
    return NULL;
  }
  if (err) {
    MJLOG_ERR("err is: %s", strerror(err));
    conn->_error = true;
    mjconn_del_cevent(conn); 
    return NULL;
  }
  // connect success
  mjconn_del_cevent(conn); 
  return NULL;
}

/*
===============================================================================
mjconn_TimeoutCallBack
  read/write timeout callback
===============================================================================
*/
static void* mjconn_cto_cb(void* arg) {
  mjconn conn = (mjconn) arg;
  conn->_timeout = true;
  mjconn_del_cevent(conn);
  return NULL;
}

/*
===============================================================================
mjconn_add_cevent
  add connect event 
===============================================================================
*/
static bool mjconn_add_cevent(mjconn conn) {
  // add to eventloop
  if (mjev_add_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE, 
        mjconn_cevent_cb, conn) < 0)  {
    MJLOG_ERR("mjev_add_fevent error");
    goto failout;
  }
  // set connect timeout
  if (conn->_cto) {
    if (conn->_cto_e) {
      MJLOG_ERR("mjconn connect event not null");
      mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE);
      goto failout;
    }
    conn->_cto_e = mjev_add_timer(conn->_ev, conn->_cto, mjconn_cto_cb, conn);
    if (!conn->_cto_e) {
      MJLOG_ERR("mjev_add_timer error");
      mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE);
      goto failout;
    }
  }
  return true;

failout:
  conn->_error = true;
  return false;
}

/*
===============================================================================
mjconn_Connect
  connect to host async
===============================================================================
*/
bool mjconn_connect(mjconn conn, const char* ipaddr, int port, mjProc CB) {
  // sanity check
  if (!conn || !CB) {
    MJLOG_ERR("conn or proc is null");
    return false;
  }
  // can't re enter
  if (conn->_ctype != MJCONN_NONE) {
    MJLOG_ERR("connectType must be MJCONN_NONE");
    return false;
  }
  // set conn type and callback
  conn->_ctype  = MJCONN_CONN;
  conn->_CCB    = CB;
  // init address
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  inet_pton(AF_INET, ipaddr, &addr.sin_addr);
  // try to connect 
  int ret = connect(conn->_fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == 0) {       
    // connect success, run callback in pending
    conn->_ctype = MJCONN_NONE;
    if (conn->_CCB) mjev_add_pending(conn->_ev, conn->_CCB, conn);
    return true;
  }
  // connect failed, set nonblock connect
  if (errno == EINPROGRESS) return mjconn_add_cevent(conn);
  MJLOG_ERR("connect failed");  
  conn->_error = true;
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
static inline mjstr mjconn_set_buffer(mjstr defVal) {
  if (defVal) return defVal;
  return mjstr_new(1024); // alloc 1K buffer
}

/*
===============================================================================
mjconn_new
  create mjconn
  return NULL -- fail, other -- success
===============================================================================
*/
mjconn mjconn_new(mjev ev, int fd) {
  if (!ev) return NULL;   
  if (fd > MAX_FD) {
    MJLOG_ERR("fd is too large");
    return NULL;
  }
  mjsock_set_blocking(fd, 0);
  // alloc mjconn struct 
  mjconn conn = &_conn[fd];
  mjstr rbak = conn->_rbuf;
  mjstr wbak = conn->_wbuf;
  mjstr dbak = conn->_data;
  // clean mjconn
  memset(conn, 0, sizeof(struct mjconn));
  conn->_fd = fd;       // set conn fd 
  conn->_ev = ev;       // set ev
  // create buffer
  conn->_rbuf = mjconn_set_buffer(rbak);
  conn->_wbuf = mjconn_set_buffer(wbak);
  conn->_data = mjconn_set_buffer(dbak);
  if (!conn->_rbuf || !conn->_wbuf || !conn->_data) {
    MJLOG_ERR("mjstr create error");
    return NULL;
  }
  // create mjmap
  conn->_map = mjmap_new(31);
  if (!conn->_map) {
    MJLOG_ERR("mjmap create error");
    return NULL;
  }
  return conn;
}

/*
===============================================================================
mjconn_delete
  delete mjconn struct
===============================================================================
*/
bool mjconn_delete(mjconn conn) {
  if (!conn) return false;
  // invalid connect timeout event
  if (conn->_cto_e) {
    mjev_del_timer(conn->_ev, conn->_cto_e);
    conn->_cto    = 0;
    conn->_cto_e  = NULL;
  }
  // invalid read timeout event
  if (conn->_rto_e) {
    mjev_del_timer(conn->_ev, conn->_rto_e); 
    conn->_rto    = 0;
    conn->_rto_e  = NULL;
  }
  // invalid write timeout event
  if (conn->_wto_e) {
    mjev_del_timer(conn->_ev, conn->_wto_e);
    conn->_wto    = 0;
    conn->_wto_e  = NULL;
  }
  // delete eventloop fd, pending proc
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE);
  mjev_del_pending(conn->_ev, conn);
  mjmap_delete(conn->_map);
  mjsock_close(conn->_fd);
  return true;
}
