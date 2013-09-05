#include "mjredis.h"
#include "mjlog.h"
#include <stdlib.h>
#include <string.h>

/*
===============================================================================
mjredis_connect
  connect to redis server
  return 
    true --- connect ok
    false --- connect failure
===============================================================================
*/
static bool mjredis_connect(mjredis handle) {
  if (handle->_context) redisFree(handle->_context);
  handle->_context = redisConnect(handle->_ip, handle->_port);
  if (!handle->_context) return false; 
  // check err code
  if (handle->_context->err) {
    redisFree(handle->_context);
    handle->_context = NULL;
    return false;
  }
  return true;
}

/*
===============================================================================
mjredis_cmd_generic
  run redis cmd
===============================================================================
*/
static redisReply* mjredis_cmd_generic(mjredis handle, int* retval, 
    const char* format, ...) {
  // link is invalid try connect
  if (!handle->_context && !mjredis_connect(handle)) {
    *retval = -2;
    return NULL;
  }
  // run command
  int retry = 1;
  va_list ap;
  redisReply* reply = NULL;
  for (;;) {
    // call redis command
    va_start(ap, format);
    reply = redisvCommand(handle->_context, format, ap);
    va_end(ap);
    // no link error break
    if (reply) break;
    if (!retry) {
      MJLOG_ERR("max retry reached");
      *retval = -2;
      return NULL;
    }
    // re connect
    if (!mjredis_connect(handle)) {
      MJLOG_ERR("reconnect failed");
      *retval = -2;
      return NULL;
    }
    retry--;
  }
//  MJLOG_ERR("type: %d, integer: %lld, str: %s", reply->type, reply->integer, reply->str);
  // check reply error
  if (reply->type == REDIS_REPLY_ERROR) {
    freeReplyObject(reply);
    *retval = -1;
    return NULL;
  }
  *retval = 0;
  return reply;
}

/*
===============================================================================
mjredis_get
  get value from redis
  return 
    -2 -- link error
    -1 -- error
    0 -- success, but no value found
    1 -- success, value in out_value
===============================================================================
*/
int mjredis_get(mjredis handle, const char* key, mjstr out_value) {
  // sanity check
  if (!handle || !key) {
    MJLOG_ERR("redis handle or key or value is null");
    return -1;
  }
  // call generic cmd
  int retval;
  redisReply* reply = mjredis_cmd_generic(handle, &retval, "GET %s", key);
  if (!reply) {
    MJLOG_ERR("mjredis get error");
    return retval;
  }
  // no value found
  if (reply->type == REDIS_REPLY_NIL) {
    freeReplyObject(reply);
    return 0;
  }
  // copy value
  if (out_value) mjstr_copyb(out_value, reply->str, reply->len);
  freeReplyObject(reply);
  return 1;
}

/*
===============================================================================
mjredis_set
  set value
  return -1 -- error
          0 -- success
===============================================================================
*/
int mjredis_set(mjredis handle, const char* key, const char* value) {
  // sanity check
  if (!handle || !key || !value) {
    MJLOG_ERR("redis handle or key or value is null");
    return -1;
  }
  int retval;
  redisReply* reply = mjredis_cmd_generic(handle, &retval, "SET %s %s", key, 
      value);
  if (!reply) {
    MJLOG_ERR("mjredis_set error");
    return retval;
  }
  freeReplyObject(reply);
  return 0;
}

/*
===============================================================================
mjredis_del
  del value
  return 
        -1 -- error
        other -- numbers of deleted key 
===============================================================================
*/
int mjredis_del(mjredis handle, const char* key) {
  // sanity check
  if (!handle || !key) {
    MJLOG_ERR("redis handle or key is null");
    return -1;
  }
  // call generic cmd
  int retval;
  redisReply* reply = mjredis_cmd_generic(handle, &retval, "DEL %s", key);
  if (!reply) {
    MJLOG_ERR("mjredis_del error");
    return retval;
  }
  // reply error check
  if (reply->type != REDIS_REPLY_INTEGER) {
    MJLOG_ERR("Oops it can't happen");
    freeReplyObject(reply);
    return -1;
  }
  // return number of deleted
  retval = reply->integer;
  freeReplyObject(reply);
  return retval;
}

int mjredis_lpush(mjredis handle, const char* key, const char* value) {
  // sanity check
  if (!handle || !key || !value) {
    MJLOG_ERR("redis handle or key or value is null");
    return -1;
  }
  // call lpush
  int retval;
  redisReply* reply = mjredis_cmd_generic(handle, &retval, "LPUSH %s %s", key, 
      value);
  if (!reply) {
    MJLOG_ERR("mjredis_lpush error");
    return retval;
  }
  freeReplyObject(reply);
  return 0;
}

/*
===============================================================================
mjredis_rpop
  rpop list
  return -2 -- link error
          -1 -- run error
          0 -- success no data
          1 -- success
===============================================================================
*/
int mjredis_rpop(mjredis handle, const char* key, mjstr out_value) {
  // sanity check
  if (!handle || !key) {
    MJLOG_ERR("redis handle or key is null");
    return -1;
  }
  // call rpop
  int retval;
  redisReply* reply = mjredis_cmd_generic(handle, &retval, "RPOP %s", key);
  if (!reply) {
    MJLOG_ERR("mjredis_rpop error");
    return retval;
  }
  // no value found
  if (reply->type == REDIS_REPLY_NIL) {
    freeReplyObject(reply);
    return 0;
  }
  if (out_value) mjstr_copyb(out_value, reply->str, reply->len);
  freeReplyObject(reply);
  return 1;
}

/*
===============================================================================
mjredis_llen
  llen key
  return -2 -- link error
          -1 -- command error
          other -- length of list
===============================================================================
*/
int mjredis_llen(mjredis handle, const char* key) {
  // sanity check
  if (!handle || !key) {
    MJLOG_ERR("redis handle or key is null");
    return -1;
  }
  // call llen
  int retval;
  redisReply* reply = mjredis_cmd_generic(handle, &retval, "LLEN %s", key);
  if (!reply) {
    MJLOG_ERR("mjredis_llen error");
    return retval;
  }
  // check result
  if (reply->type != REDIS_REPLY_INTEGER) {
    MJLOG_ERR("Oops llen result is not integer");
    freeReplyObject(reply);
    return -1;
  }
  retval = reply->integer;
  freeReplyObject(reply);
  return retval;
}

/*
===============================================================================
mjredis_select
  select db
  return -2 -- link error
         -1 -- command error
         0 -- success
===============================================================================
*/
int mjredis_select(mjredis handle, const char* db) {
  // sanity check
  if (!handle || !db) {
    MJLOG_ERR("redis handle or db is null");
    return -1;
  }
  // call llen
  int retval;
  redisReply* reply = mjredis_cmd_generic(handle, &retval, "SELECT %s", db);
  if (!reply) {
    MJLOG_ERR("mjredis_select error");
    return retval;
  }
  freeReplyObject(reply);
  return 0;
}

/*
===============================================================================
mjredis_new
  create new mjredis
===============================================================================
*/
mjredis mjredis_new(const char* ip, int port) {
  mjredis handle = (mjredis) calloc(1, sizeof(struct mjredis));
  if (!handle) return NULL;
  strcpy(handle->_ip, ip); 
  handle->_port = port;
  // connect to redis
  if (!mjredis_connect(handle)) {
    free(handle);
    return NULL;
  }
  return handle;
}

/*
===============================================================================
mjredis_delete
  delete mjredis
===============================================================================
*/
bool mjredis_delete(mjredis handle) {
  if (!handle) return false;
  if (handle->_context) redisFree(handle->_context);
  free(handle);
  return true;
}
