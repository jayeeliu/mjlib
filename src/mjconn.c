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
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE);
  if (conn->_read_timeout) {
    // invalid timer event
    mjev_del_timer(conn->_ev, conn->_read_timeout_event);
    conn->_read_timeout     	= 0;
    conn->_read_timeout_event	= NULL;
  }
  // reset read type
  conn->_read_type = MJCONN_NONE;
  // run callback in the last step
  if (conn->_ReadCallBack) conn->_ReadCallBack(conn);
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
    int ret = read(conn->_fd, buf, BUF_SIZE);
    if (ret == -1) {          
      // interrupt by signal continue
      if (errno == EINTR) continue;
      // nonblock, no data, break
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
  if (conn->_read_type == MJCONN_READBYTES) {
    // readtype is readbytes
    if (conn->_rbytes <= conn->_rbuf->length) { 
      mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbytes);
      mjstr_consume(conn->_rbuf, conn->_rbytes);
      mjconn_del_read_event(conn);
      return NULL;
    }
  } else if (conn->_read_type == MJCONN_READUNTIL) { 
    // read type is readuntil
    int pos = mjstr_search(conn->_rbuf, conn->_delim);
    if (pos != -1) {
      mjstr_copyb(conn->_data, conn->_rbuf->data, pos);
      mjstr_consume(conn->_rbuf, pos + strlen(conn->_delim));
      mjconn_del_read_event(conn);
      return NULL;
    }
  } else if (conn->_read_type == MJCONN_READ) { 
    // read type is normal read
    if (conn->_rbuf && conn->_rbuf->length > 0) {
      mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbuf->length);
      mjstr_consume(conn->_rbuf, conn->_rbuf->length);
      mjconn_del_read_event(conn);
      return NULL;
    }
  }
  // some error happend, close conn
  if (conn->_closed || conn->_error) {
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
  if (conn->_closed || conn->_error) {
    MJLOG_ERR("conn is closed or error");
    goto failout;
  }
  // add readevent
  if (mjev_add_fevent(conn->_ev, conn->_fd, MJEV_READABLE, 
    mjconn_read_event_callback, conn) < 0) {
    MJLOG_ERR("mjev_Add error");
    goto failout;
  }
  // add read timeout event
  if (conn->_read_timeout) {
    conn->_read_timeout_event = mjev_add_timer(conn->_ev, conn->_read_timeout, 
        mjconn_timeout_callback, conn);
    if (!conn->_read_timeout_event) {
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
  if (conn->_read_type != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->_read_type       = MJCONN_READBYTES;    
  conn->_rbytes          = len;
  conn->_ReadCallBack    = CallBack;
  // check rbuf
  if (conn->_rbytes <= conn->_rbuf->length) { 
    // copy rbytes to data
    mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbytes);
    mjstr_consume(conn->_rbuf, conn->_rbytes);
    // read finish
    conn->_read_type = MJCONN_NONE;
    // run callback
    if (conn->_ReadCallBack) {
      mjev_add_pending(conn->_ev, conn->_ReadCallBack(conn), conn);
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
  if (conn->_read_type != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->_read_type    = MJCONN_READUNTIL;
  conn->_delim     		= delim;
  conn->_ReadCallBack	= CallBack;
  // found data in rbuf, call proc and return 
  int pos = mjstr_search(conn->_rbuf, conn->_delim);
  if (pos != -1) {
    // copy data to rbuf, not include delim
    mjstr_copyb(conn->_data, conn->_rbuf->data, pos);
    mjstr_consume(conn->_rbuf, pos + strlen(conn->_delim));
    // read finish set readType to NONE, run callback
    conn->_read_type = MJCONN_NONE;
    // run read callback
    if (conn->_ReadCallBack) {
      mjev_add_pending(conn->_ev, conn->_ReadCallBack(conn), conn);
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
  if (conn->_read_type != MJCONN_NONE) {
    MJLOG_ERR("readType must be MJCONN_NONE");
    return false;
  }
  // set read type
  conn->_read_type   	= MJCONN_READ;
  conn->_ReadCallBack	= CallBack;
  // found data in rbuf
  if (conn->_rbuf && conn->_rbuf->length > 0) {
    mjstr_copyb(conn->_data, conn->_rbuf->data, conn->_rbuf->length);
    mjstr_consume(conn->_rbuf, conn->_rbuf->length);
    conn->_read_type  = MJCONN_NONE;
    if (conn->_ReadCallBack) {
      mjev_add_pending(conn->_ev, conn->_ReadCallBack(conn), conn);
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
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_WRITEABLE);
  // del write timeout event
  if (conn->_write_timeout) {
    mjev_del_timer(conn->_ev, conn->_write_timeout_event);
    conn->_write_timeout    		= 0;  
    conn->_write_timeout_event	= NULL;
  }
  // set write type to NONE
  conn->_write_type = MJCONN_NONE;
  // call write callback
  if (conn->_WriteCallBack) conn->_WriteCallBack(conn);
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
  int ret = write(conn->_fd, conn->_wbuf->data, conn->_wbuf->length);
  if (ret < 0) {
    MJLOG_ERR("conn write error: %s", strerror(errno));
    mjconn_delete(conn);
    return NULL;
  }
  mjstr_consume(conn->_wbuf, ret);
  // no data to write call DelWriteEvent
  if (conn->_wbuf->length == 0) mjconn_del_write_event(conn);
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
  if (conn->_closed || conn->_error) {
    MJLOG_ERR("conn is closed or error");
    goto failout;
  }
  // add write event
  if (mjev_add_fevent(conn->_ev, conn->_fd, MJEV_WRITEABLE, 
      mjconn_write_event_callback, conn) < 0) {
    MJLOG_ERR("mjEV_Add error");
    goto failout;
  }
  // AddWriteEvent can be call many times
  // When we call it twice, we can't change the callback
  if (conn->_write_timeout && !conn->_write_timeout_event) { 
    conn->_write_timeout_event = mjev_add_timer(conn->_ev, 
        conn->_write_timeout, mjconn_timeout_callback, conn);
    if (!conn->_write_timeout_event) {
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
  mjstr_cats(conn->_wbuf, buf);
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
  conn->_WriteCallBack = CallBack;
  // if re enter only change callback
  if (conn->_write_type == MJCONN_WRITE) return true;
  conn->_write_type = MJCONN_WRITE;
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
  conn->_connect_timeout = connect_timeout;
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
  conn->_read_timeout   = read_timeout;
  conn->_write_timeout  = write_timeout;
  return true;
}

/*
===============================================================================
mjconn_get_obj
	get object from mjconn
===============================================================================
*/
void* mjconn_get_obj(mjconn conn, const char* key) {
	if (!conn || !key) {
		MJLOG_ERR("conn or key is null");
		return NULL;
	}
	return mjmap_get_obj(conn->_arg_map, key);
}

/*
===============================================================================
mjconn_set_obj
	set object to mjconn
===============================================================================
*/
bool mjconn_set_obj(mjconn conn, const char* key, void* obj, mjProc obj_free) {
	if (!conn || !key) {
		MJLOG_ERR("conn or key is null");
		return false;
	}
	if (mjmap_set_obj(conn->_arg_map, key, obj, obj_free) < 0) return false;
	return true;
}

/*
===============================================================================
mjconn_DelConnectEvent
  del connect event
===============================================================================
*/
static void* mjconn_del_connect_event(mjconn conn) {
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE); 
  if (conn->_connect_timeout) {
    mjev_del_timer(conn->_ev, conn->_connect_timeout_event);
    conn->_connect_timeout    		= 0;
    conn->_connect_timeout_event	= NULL;
  }
  conn->_connect_type = MJCONN_NONE;  
  if (conn->_ConnectCallback) conn->_ConnectCallback(conn);
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
  if (getsockopt(conn->_fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
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
  if (mjev_add_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE, 
        mjconn_connect_event_callback, conn) < 0)  {
    MJLOG_ERR("mjEV_Add error");
    goto failout;
  }
  // set connect timeout
  if (conn->_connect_timeout) {
    conn->_connect_timeout_event = mjev_add_timer(conn->_ev, 
        conn->_connect_timeout, mjconn_timeout_callback, conn);
    if (!conn->_connect_timeout_event) {
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
  if (conn->_connect_type != MJCONN_NONE) {
    MJLOG_ERR("connectType must be MJCONN_NONE");
    return false;
  }
  // set conn type and callback
  conn->_connect_type    = MJCONN_CONN;
  conn->_ConnectCallback  = CallBack;
  // init address
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  inet_pton(AF_INET, ipaddr, &addr.sin_addr);
  // try to connect 
  int ret = connect(conn->_fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == 0) {       
    // connect success
    conn->_connect_type = MJCONN_NONE;
    if (conn->_ConnectCallback) {
      mjev_add_pending(conn->_ev, conn->_ConnectCallback, conn);
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
  return mjstr_new(1024);
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
	conn->_arg_map = mjmap_new(31);
	if (!conn->_arg_map) {
		MJLOG_ERR("mjmap create error");
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
  if (conn->_connect_timeout) {
    mjev_del_timer(conn->_ev, conn->_connect_timeout_event);
    conn->_connect_timeout    		= 0;
    conn->_connect_timeout_event	= NULL;
  }
  // invalid read timeout event
  if (conn->_read_timeout) {
    mjev_del_timer(conn->_ev, conn->_read_timeout_event); 
    conn->_read_timeout     	= 0;
    conn->_read_timeout_event	= NULL;
  }
  // invalid write timeout event
  if (conn->_write_timeout) {
    mjev_del_timer(conn->_ev, conn->_write_timeout_event);
    conn->_write_timeout    		= 0;
    conn->_write_timeout_event	= NULL;
  }
  // delete eventloop fd, pending proc
  mjev_del_fevent(conn->_ev, conn->_fd, MJEV_READABLE | MJEV_WRITEABLE);
  mjev_del_pending(conn->_ev, conn);
	mjmap_delete(conn->_arg_map);
  mjsock_close(conn->_fd);
  return true;
}
