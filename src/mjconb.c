#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include "mjconb.h"
#include "mjlog.h"
#include "mjsock.h"

#define MJCONB_NONE      0
#define MJCONB_READ      1
#define MJCONB_READBYTES 2 
#define MJCONB_READUNTIL 3

#define BUF_SIZE      4096

/*
===============================================================================
mjconb_ReadToBuf
  read data to buffer
  return  -3 --- too bad, should not be this
      -2 --- readtimeout, get some data
      -1 --- readerror, get some data
       0 --- peer close, get some data
      other --- get data
===============================================================================
*/
static int mjconb_read_to_buf(mjconb conn, mjstr data) {
  int ret = -3;
  char buf[BUF_SIZE];
  // read data first in a loop
  for (;;) {
    // buffer has enough data, copy and return
    if (conn->_rtype == MJCONB_READBYTES) {
      if (conn->_rbytes <= conn->_rbuf->len) { 
        mjstr_copyb(data, conn->_rbuf->data, conn->_rbytes);
        mjstr_consume(conn->_rbuf, conn->_rbytes);
        return data->len;
      }
    } else if (conn->_rtype == MJCONB_READUNTIL) {
      int pos = mjstr_search(conn->_rbuf, conn->_delim);
      if (pos != -1) {
        mjstr_copyb(data, conn->_rbuf->data, pos);
        mjstr_consume(conn->_rbuf, pos + strlen(conn->_delim));
        return data->len + strlen(conn->_delim);
      }
    } else if (conn->_rtype == MJCONB_READ) {
      if (conn->_rbuf && conn->_rbuf->len > 0) {
        mjstr_copyb(data, conn->_rbuf->data, 
            conn->_rbuf->len);
        mjstr_consume(conn->_rbuf, conn->_rbuf->len);
        return data->len;
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
    mjstr_catb(conn->_rbuf, buf, ret);
  }
  // read error or read close, copy data
  mjstr_copy(data, conn->_rbuf);
  mjstr_consume(conn->_rbuf, conn->_rbuf->len);
  return ret;
}

/*
===============================================================================
mjconb_read
  read data normal
===============================================================================
*/
int mjconb_read(mjconb conn, mjstr buf) {
  if (!conn || !buf || conn->_closed || conn->_error) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  conn->_rtype = MJCONB_READ;
  return mjconb_read_to_buf(conn, buf);
}

/* 
===============================================================================
mjconb_readbytes
  read len size bytes 
===============================================================================
*/
int mjconb_readbytes(mjconb conn, mjstr buf, unsigned int len) {
  if (!conn || !buf || len <= 0 || conn->_closed || conn->_error) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  conn->_rtype  = MJCONB_READBYTES;    
  conn->_rbytes = len;
  return mjconb_read_to_buf(conn, buf);
}

/* 
===============================================================================
mjconb_readuntil
  read data until delim 
===============================================================================
*/
int mjconb_readuntil(mjconb conn, const char* delim, mjstr buf) {
  if (!conn || !buf || !delim || conn->_closed || conn->_error) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  conn->_rtype  = MJCONB_READUNTIL;
  conn->_delim  = delim;
  return mjconb_read_to_buf(conn, buf);
}

/*
===============================================================================
mjconb_write
  write data to conn
===============================================================================
*/
int mjconb_write(mjconb conn, mjstr buf) {
  if (!conn || !buf || !buf->len || conn->_closed || conn->_error) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  return mjconb_writeb(conn, buf->data, buf->len);
}

/*
===============================================================================
mjconb_WriteB
  write data to conn
  return: -1 --- write error
      -2 --- write timeout
      other --- write data
===============================================================================
*/
int mjconb_writeb(mjconb conn, char *buf , unsigned int len) {
  if (!conn || !buf || !len || conn->_closed || conn->_error) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  int total_write = 0;
  while (total_write < len) {
    int ret = write(conn->_fd, buf + total_write, len);
    if (ret == -1) {
      MJLOG_ERR("mjconb Write Error");
      if (errno == EAGAIN || errno == EWOULDBLOCK) ret = -2;
      conn->_error = true;
      return ret;
    }
    if (!ret) {
      MJLOG_ERR("nothing write");
    }
    total_write += ret;
  }
  return total_write;
}

/*
===============================================================================
mjconb_WriteS
  write string call mjconb_WriteB
===============================================================================
*/
int mjconb_writes(mjconb conn, char *buf) {
  if (!conn || !buf || conn->_closed || conn->_error) {
    MJLOG_ERR("sanity check error");
    return -1;
  }
  return mjconb_writeb(conn, buf, strlen(buf));
}

/*
===============================================================================
mjconb_get_obj
  get object from conn
===============================================================================
*/
void* mjconb_get_obj(mjconb conn, const char* key) {
  if (!conn || !key) {
    MJLOG_ERR("conn or key is null");
    return NULL;
  }
  return mjmap_get_obj(conn->_map, key);
}

/*
===============================================================================
mjconn_set_obj
  mjconn set object
===============================================================================
*/
bool mjconb_set_obj(mjconb conn, const char* key, void* obj, mjProc obj_free) {
  if (!conn || !key) {
    MJLOG_ERR("conn or key is null");
    return false;
  }
  if (mjmap_set_obj(conn->_map, key, obj, obj_free) < 0) return false;
  return true;
}

/*
===============================================================================
mjconb_set_timeout
  set mjconb read and write timeout
===============================================================================
*/
bool mjconb_set_timeout(mjconb conn, unsigned int rto, unsigned int wto) {
  if (!conn) {
    MJLOG_ERR("conn is null");
    return false;
  }
  if (rto) {
    struct timeval tv;
    tv.tv_sec   = rto / 1000;
    tv.tv_usec  = (rto % 1000) * 1000;
    if (setsockopt(conn->_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
      MJLOG_ERR("setsockopt SO_RCVTIMEO error");
      return false;
    }
  }
  if (wto) {
    struct timeval tv;
    tv.tv_sec   = wto / 1000;
    tv.tv_usec  = (wto % 1000) * 1000;
    if (setsockopt(conn->_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
      MJLOG_ERR("setsockopt SO_SNDTIMEO error");
      return false;
    }
  }
  return true;
}

#define MAX_FD        60000
static struct mjconb  _conn[MAX_FD];

/*
===============================================================================
mjconb_new
  input socket fd, output mjconb struct
  return NULL -- fail, other -- success
===============================================================================
*/
mjconb mjconb_new(int fd) {
  if (fd >= MAX_FD) {
    MJLOG_ERR("fd too large");
    return NULL;
  }
  if (!mjsock_set_blocking(fd, 1)) {
    MJLOG_ERR("mjsock_set_blocking error");
    return NULL;
  }
  // get mjconb struct
  mjconb conn = &_conn[fd];
  conn->_fd   = fd;      
  // create read_buf
  if (!conn->_rbuf) {
    // create read buffer
    conn->_rbuf = mjstr_new(128);
    if (!conn->_rbuf) {
      MJLOG_ERR("mjstr create error");
      return NULL;
    }
  }
  mjstr_clean(conn->_rbuf);
  // init read
  conn->_rtype  = MJCONB_NONE;
  conn->_delim  = NULL;
  conn->_rbytes = -1;
  conn->_timeout = conn->_error = conn->_closed = false;
  // init mjmap
  conn->_map = mjmap_new(31);
  if (!conn->_map) {
    MJLOG_ERR("mjmap_new error");
    return NULL;
  }
  return conn;
}

/*
===============================================================================
mjconb_connect_ready
  Used by mjconb_Connect for connect timeout
  return  0 -- connect timeout or poll failed
          1 -- connect ok
===============================================================================
*/
static int mjconb_connect_ready(int fd, unsigned int timeout) {
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
mjconb_Connect
  conn to addr and port
  return NULL -- fail, other -- success
===============================================================================
*/
mjconb mjconb_connect(const char *addr, int port, unsigned int timeout) {
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
      if (!mjconb_connect_ready(fd, timeout)) {
        MJLOG_ERR("conn timeout");
        close(fd);
        continue;
      }
    } 
    // connect ok return mjconb
    return mjconb_new(fd);
  }
  return NULL;
}

/*
===============================================================================
mjconb_delete
  delete conn struct
===============================================================================
*/
bool mjconb_delete(mjconb conn) {
  if (!conn) return false;
  mjmap_delete(conn->_map);
  mjsock_close(conn->_fd);
  return true;
}
