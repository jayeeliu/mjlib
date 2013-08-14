#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "mjlog.h"
#include "mjio.h"

#define BUF_LEN 1024

/*
===============================================================================
mjIO_Read
  return mj file io
===============================================================================
*/
int mjIO_Read(mjIO io, mjstr data, int len) {
  // sanity check
  if (!io || !data || len <= 0) return 0;
  // rbuf has data return
  if (io->rbuf && io->rbuf->length > 0) {
    mjstr_copy(data, io->rbuf);
    mjstr_consume(io->rbuf, io->rbuf->length);
    return data->length;
  }
  // io rbuf is empty
  if (len > BUF_LEN) len = BUF_LEN;
  char buf[BUF_LEN];
  int ret = read(io->fd, buf, len);
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
mjIO_ReadLine
  read data from io
  return -1 -- read file error, has data
       -2 -- file close, has data 
       -3 -- can't be this
===============================================================================
*/
int mjIO_ReadLine(mjIO io, mjstr data) {  
  int ret = -3;
  char buf[BUF_LEN];

  while (1) {
    int pos = mjstr_search(io->rbuf, "\n");
    if (pos != -1) {
      mjstr_copyb(data, io->rbuf->data, pos + 1);
      mjstr_consume(io->rbuf, pos + 1);
      return data->length;
    }
    // get data from file
    ret = read(io->fd, buf, BUF_LEN);
    if (ret < 0) {
      ret = -1;
      MJLOG_ERR("data read error");
      break;
    } else if (!ret) {
      ret = -2;
      break;
    }
    mjstr_catb(io->rbuf, buf, ret);
  }
  mjstr_copy(data, io->rbuf);
  mjstr_consume(io->rbuf, io->rbuf->length);
  return ret;
}

/*
===============================================================================
mjIO_New
  creat mjIO struct
===============================================================================
*/
mjIO mjIO_New(const char* fileName) {
  // alloc mjio struct
  mjIO io = (mjIO) calloc(1, sizeof(struct mjIO));
  if (!io) {
    MJLOG_ERR("mjio alloc error");
    goto failout1;
  }
  // get fileName open file
  io->fileName = fileName;
  io->fd = open(io->fileName, O_RDWR);
  if (io->fd < 0) {
    MJLOG_ERR("open file error");
    goto failout2;
  }
  // alloc mjio rbuf
  io->rbuf = mjstr_new();
  if (!io->rbuf) {
    MJLOG_ERR("mjstr_New error");
    goto failout3;
  }
  return io;

failout3:
  close(io->fd);
failout2:
  free(io);
failout1:
  return NULL;
}

/*
===============================================================================
mjIO_Delete
  delete mjio struct
===============================================================================
*/
bool mjIO_Delete(mjIO io) {
  if (!io) {
    MJLOG_ERR("io is null");
    return false;
  }
  close(io->fd);
  mjstr_delete(io->rbuf);
  free(io);
  return true;
}
