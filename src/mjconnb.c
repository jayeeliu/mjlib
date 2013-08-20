#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include "mjconnb.h"
#include "mjlog.h"
#include "mjsock.h"

#define MJCONNB_NONE      0
#define MJCONNB_READ      1
#define MJCONNB_READBYTES 2 
#define MJCONNB_READUNTIL 3

#define BUF_SIZE      4096

/*
===============================================================================
mjconnb_ReadToBuf
  read data to buffer
  return  -3 --- too bad, should not be this
      -2 --- readtimeout, get some data
      -1 --- readerror, get some data
       0 --- peer close, get some data
      other --- get data
===============================================================================
*/
static int mjconnb_read_to_buf(mjconnb conn, mjstr data) {
  int ret = -3;
  char buf[BUF_SIZE];
	// read data first in a loop
  for (;;) {
    // buffer has enough data, copy and return
    if (conn->_readtype == MJCONNB_READBYTES) {
      if (conn->_rbytes <= conn->_read_buf->length) { 
        mjstr_copyb(data, conn->_read_buf->data, conn->_rbytes);
        mjstr_consume(conn->_read_buf, conn->_rbytes);
        return data->length;
      }
    } else if (conn->_readtype == MJCONNB_READUNTIL) {
      int pos = mjstr_search(conn->_read_buf, conn->_delim);
      if (pos != -1) {
        mjstr_copyb(data, conn->_read_buf->data, pos);
        mjstr_consume(conn->_read_buf, pos + strlen(conn->_delim));
        return data->length;
      }
    } else if (conn->_readtype == MJCONNB_READ) {
      if (conn->_read_buf && conn->_read_buf->length > 0) {
        mjstr_copyb(data, conn->_read_buf->data, conn->_read_buf->length);
        mjstr_consume(conn->_read_buf, conn->_read_buf->length);
        return data->length;
      }
    }
    // we must read data  
    ret = read(conn->_fd, buf, BUF_SIZE);
    if (ret == -1) {           
      // intrrupt by signal try again
      if (errno == EINTR) continue;           
      // read timeout, set ret and break
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        MJLOG_ERR("read timeout");
        conn->_timeout = true;
        ret = -2;
      } else {
        conn->_error = true;
      }
      break;         
    }
    // read close, break, copy data to read_buf
    if (ret == 0) {
      MJLOG_ERR("conn close");
      conn->_closed = true;
      break;
    }
    // read ok put data to read_buf, try again
    mjstr_catb(conn->_read_buf, buf, ret);
  }
  // read error or read close, copy data
  mjstr_copy(data, conn->_read_buf);
  mjstr_consume(conn->_read_buf, conn->_read_buf->length);
  return ret;
}

/*
===============================================================================
mjconnb_Read
  read data normal
===============================================================================
*/
int mjconnb_read(mjconnb conn, mjstr data) {
  // sanity check
  if (!conn || !data) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  conn->_readtype = MJCONNB_READ;
  return mjconnb_read_to_buf(conn, data);
}

/* 
===============================================================================
mjconnb_ReadBytes
  read len size bytes 
===============================================================================
*/
int mjconnb_readbytes(mjconnb conn, mjstr data, int len) {
  // sanity check
  if (!conn || !data || len <= 0) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  conn->_readtype  = MJCONNB_READBYTES;    
  conn->_rbytes    = len;
  return mjconnb_read_to_buf(conn, data);
}

/* 
===============================================================================
mjconnb_ReadUntil
  read data until delim 
===============================================================================
*/
int mjconnb_readuntil(mjconnb conn, const char* delim, mjstr data) {
  // sanity check
  if (!conn || !data || !delim) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  conn->_readtype  = MJCONNB_READUNTIL;
  conn->_delim     = delim;
  return mjconnb_read_to_buf(conn, data);
}

/*
===============================================================================
mjconnb_Write
  write data to conn
===============================================================================
*/
int mjconnb_write(mjconnb conn, mjstr data) {
  if (!conn || !data || !data->length) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  return mjconnb_writeb(conn, data->data, data->length);
}

/*
===============================================================================
mjconnb_WriteB
  write data to conn
  return: -1 --- write error
      -2 --- write timeout
      other --- write data
===============================================================================
*/
int mjconnb_writeb(mjconnb conn, char *buf , int length) {
  if (!conn || !buf || !length) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  int ret = write(conn->_fd, buf, length);
  if (ret == -1) {
    MJLOG_ERR("mjconnb Write Error");
    if (errno == EAGAIN || errno == EWOULDBLOCK) ret = -2;
    conn->_error = true;
  }
  if (!ret) {
    MJLOG_ERR("nothing write");
  }
  return ret;
}

/*
===============================================================================
mjconnb_WriteS
  write string call mjconnb_WriteB
===============================================================================
*/
int mjconnb_writes(mjconnb conn, char *buf) {
  if (!conn || !buf) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  return mjconnb_writeb(conn, buf, strlen(buf));
}

/*
===============================================================================
mjconnb_get_obj
  get object from conn
===============================================================================
*/
void* mjconnb_get_obj(mjconnb conn, const char* key) {
  if (!conn || !key) {
    MJLOG_ERR("conn or key is null");
    return NULL;
  }
  return mjmap_get_obj(conn->_arg_map, key);
}

/*
===============================================================================
mjconn_set_obj
  mjconn set object
===============================================================================
*/
bool mjconnb_set_obj(mjconnb conn, const char* key, void* obj,
    mjProc obj_free) {
  if (!conn || !key) {
    MJLOG_ERR("conn or key is null");
    return false;
  }
  if (mjmap_set_obj(conn->_arg_map, key, obj, obj_free) < 0) return false;
  return true;
}

/*
===============================================================================
mjconnb_SetTimeout
  set mjconnb read and write timeout
  return    -1 -- set error
        0  -- success
===============================================================================
*/
bool mjconnb_set_timeout(mjconnb conn, unsigned int readTimeout, 
      unsigned int writeTimeout) {
  // sanity check
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  //set read timeout  
  if (readTimeout) {
    struct timeval tv;
    tv.tv_sec   = readTimeout / 1000;
    tv.tv_usec  = (readTimeout % 1000) * 1000;
    // set recv timeout
    if (setsockopt(conn->_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
      MJLOG_ERR("setsockopt error");
      return false;
    }
  }
  if (writeTimeout) {
    struct timeval tv;
    tv.tv_sec   = writeTimeout / 1000;
    tv.tv_usec  = (writeTimeout % 1000) * 1000;
    // set send timeout
    if (setsockopt(conn->_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
      MJLOG_ERR("setsockopt error");
      return false;
    }
  }
  return true;
}

// conn buffer
#define MAX_FD        60000
static struct mjconnb _conn[MAX_FD];

/*
===============================================================================
mjconnb_New
  input socket fd, output mjconnb struct
  return NULL -- fail, other -- success
===============================================================================
*/
mjconnb mjconnb_new(int fd) {
  // sanity check
  if (fd >= MAX_FD) {
    MJLOG_ERR("fd is too large");
    return NULL;
  }
  // set fd to block
  if (!mjsock_set_blocking(fd, 1)) {
    MJLOG_ERR("mjsock_set_blocking error");
    return NULL;
  }
  // get mjconnb struct
  mjconnb conn	= &_conn[fd];
  conn->_fd    	= fd;      
  // create read_buf
  if (!conn->_read_buf) {
    // create read buffer
    conn->_read_buf = mjstr_new();
    if (!conn->_read_buf) {
      MJLOG_ERR("mjstr create error");
      return NULL;
    }
  }
  conn->_read_buf->length = 0;
  // init read
  conn->_readtype  = MJCONNB_NONE;
  conn->_delim     = NULL;
  conn->_rbytes    = -1;
  // init mjmap
  conn->_arg_map = mjmap_new(31);
  if (!conn->_arg_map) {
    MJLOG_ERR("mjmap_new error");
    return NULL;
  }
  // init flag
  conn->_timeout = conn->_error = conn->_closed = false;
  return conn;
}

/*
===============================================================================
mjconnb_connect_ready
  Used by mjconnb_Connect for connect timeout
  return  0 -- connect timeout or poll failed
          1 -- connect ok
===============================================================================
*/
static int mjconnb_connect_ready(int fd, unsigned int timeout) {
  // set poolfd
  struct pollfd wfd[1];
  wfd[0].fd     = fd; 
  wfd[0].events = POLLOUT;
  // set connect timeout value
  long msc = -1;
  if (timeout) msc = timeout;
  // pool for wait
  int ret = poll(wfd, 1, msc);
  if (ret == -1) {
    MJLOG_ERR("poll error");
    return 0;
  }
  if (ret == 0) {
    MJLOG_ERR("poll timeout");
    return 0;
  } 
  // connect success
  return 1;
}


/*
===============================================================================
mjconnb_Connect
  conn to addr and port
  return NULL -- fail, other -- success
===============================================================================
*/
mjconnb mjconnb_connect(const char *addr, int port, unsigned int timeout) {
  // gen port
  char _port[6];
  snprintf(_port, 6, "%d", port);
  // gen address hints
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  // get address info into servinfo
  struct addrinfo *servinfo;
  int rv = getaddrinfo(addr, _port, &hints, &servinfo);
  if (rv != 0) {
    MJLOG_ERR("getaddrinfo called error");
    return NULL;
  }
  // for each address
  struct addrinfo *p;
  for (p = servinfo; p != NULL; p = p->ai_next) {
    // get socket
    int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd == -1) {
      MJLOG_ERR("socket error");
      continue;
    }
    // set to nonblock
    mjsock_set_blocking(fd, 0);
    // try to connect
    if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
      if (errno == EHOSTUNREACH) {
        close(fd);
        continue;
      } else if (errno != EINPROGRESS) {
        close(fd);
        continue;
      }
      // now, errno == EINPROGRESS
      if (!mjconnb_connect_ready(fd, timeout)) {
        MJLOG_ERR("conn timeout");
        close(fd);
        continue;
      }
    } 
    // connect ok return mjconnb
    return mjconnb_new(fd);
  }
  return NULL;
}

/*
===============================================================================
mjconnb_Delete
  delete conn struct
  no return
===============================================================================
*/
bool mjconnb_delete(mjconnb conn) {
  // sanity check
  if (!conn) return false;
  mjmap_delete(conn->_arg_map);
  mjsock_close(conn->_fd);
  return true;
}
