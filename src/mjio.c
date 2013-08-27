#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "mjlog.h"
#include "mjio.h"

#define BUF_LEN 4096

/*
===============================================================================
mjio_Read
  return mj file io
===============================================================================
*/
int mjio_read(mjio io, mjstr data, int len) {
  // sanity check
  if (!io || !data || len <= 0) return 0;
  // rbuf has data return
  if (io->_read_buf && io->_read_buf->length > 0) {
    mjstr_copy(data, io->_read_buf);
    mjstr_consume(io->_read_buf, io->_read_buf->length);
    return data->length;
  }
  // io rbuf is empty
  if (len > BUF_LEN) len = BUF_LEN;
  char buf[BUF_LEN];
  int ret = read(io->_fd, buf, len);
  if (ret < 0) {
    ret = -1;
    MJLOG_ERR("data read error");
  } else if (!ret) {
    ret = 2;
  } else {
    mjstr_copyb(data, buf, ret);
  }
  return ret;
}

/*
===============================================================================
mjio_ReadLine
  read data from io
  return -1 -- read file error, has data
       -2 -- file close, has data 
       -3 -- can't be this
===============================================================================
*/
int mjio_readline(mjio io, mjstr data) {  
  int ret = -3;
  char buf[BUF_LEN];
	// read data
  while (1) {
    int pos = mjstr_search(io->_read_buf, "\n");
    if (pos != -1) {
      mjstr_copyb(data, io->_read_buf->data, pos + 1);
      mjstr_consume(io->_read_buf, pos + 1);
      return data->length;
    }
    // get data from file
    ret = read(io->_fd, buf, BUF_LEN);
    if (ret < 0) {
      ret = -1;
      MJLOG_ERR("data read error");
      break;
    } else if (!ret) {
      ret = -2;
      break;
    }
    mjstr_catb(io->_read_buf, buf, ret);
  }
	// here, error happends
  mjstr_copy(data, io->_read_buf);
  mjstr_consume(io->_read_buf, io->_read_buf->length);
  return ret;
}

/*
===============================================================================
mjio_New
  creat mjio struct
===============================================================================
*/
mjio mjio_new(const char* fileName) {
  // alloc mjio struct
  mjio io = (mjio) calloc(1, sizeof(struct mjio));
  if (!io) {
    MJLOG_ERR("mjio alloc error");
    goto failout1;
  }
  // get fileName open file
  io->_fd = open(fileName, O_RDWR);
  if (io->_fd < 0) {
    MJLOG_ERR("open file error");
    goto failout2;
  }
  // alloc mjio rbuf
  io->_read_buf = mjstr_new(128);
  if (!io->_read_buf) {
    MJLOG_ERR("mjstr_New error");
    goto failout3;
  }
  return io;

failout3:
  close(io->_fd);
failout2:
  free(io);
failout1:
  return NULL;
}

/*
===============================================================================
mjio_Delete
  delete mjio struct
===============================================================================
*/
bool mjio_delete(mjio io) {
  if (!io) {
    MJLOG_ERR("io is null");
    return false;
  }
  close(io->_fd);
  mjstr_delete(io->_read_buf);
  free(io);
  return true;
}
