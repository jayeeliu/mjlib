#include "mjredis.h"
#include "mjlog.h"
#include <stdlib.h>

/*
===============================================================================
mjredis_get
  get value from redis
  return 
    -1 -- error
    0 -- success, but no value found
    1 -- success, value in out_value
===============================================================================
*/
int mjredis_get(mjredis redis_handle, const char* key, mjstr out_value) {
  // sanity check
  if (!redis_handle || !key) {
    MJLOG_ERR("redis handle or key or value is null");
    return -1;
  }
  // call get
  redisReply* reply = redisCommand(redis_handle->context, "GET %s", key);
  if (!reply || reply->type == REDIS_REPLY_ERROR) {
    MJLOG_ERR("mjredis_get error");
    freeReplyObject(reply);
    return -1;
  }
  // no value found
  if (reply->type == REDIS_REPLY_NIL) {
    freeReplyObject(reply);
    return 0;
  }
  // copy value
  if (out_value) {
    mjstr_copyb(out_value, reply->str, reply->len);
  }
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
int mjredis_set(mjredis redis_handle, const char* key, const char* value) {
  // sanity check
  if (!redis_handle || !key || !value) {
    MJLOG_ERR("redis handle or key or value is null");
    return -1;
  }
  // call and return
  redisReply* reply = redisCommand(redis_handle->context, "SET %s %s", key,
      value);
  if (!reply || reply->type == REDIS_REPLY_ERROR) {
    MJLOG_ERR("mjredis_set error");
    freeReplyObject(reply);
    return -1;
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
int mjredis_del(mjredis redis_handle, const char* key) {
  // sanity check
  if (!redis_handle || !key) {
    MJLOG_ERR("redis handle or key is null");
    return -1;
  }
  // call del
  redisReply* reply = redisCommand(redis_handle->context, "DEL %s", key);
  if (!reply || reply->type == REDIS_REPLY_ERROR) {
    MJLOG_ERR("mjredis_del error");
    freeReplyObject(reply);
    return -1;
  }
  if (reply->type != REDIS_REPLY_INTEGER) {
    MJLOG_ERR("Oops it can't happen");
    freeReplyObject(reply);
    return -1;
  }
  int retval = reply->integer;
  freeReplyObject(reply);
  return retval;
}

/*
===============================================================================
mjredis_new
  create new mjredis
===============================================================================
*/
mjredis mjredis_new(const char* ip, int port) {
  mjredis redis_handle = (mjredis) calloc(1, sizeof(struct mjredis));
  if (!redis_handle) return NULL;
  // connect to redis
  redis_handle->context = redisConnect(ip, port);
  if (!redis_handle->context) {
    free(redis_handle);
    return NULL;
  }
  // connect error ?
  if (redis_handle->context->err) {
    redisFree(redis_handle->context);
    free(redis_handle);
    return NULL;
  }
  return redis_handle;
}

/*
===============================================================================
mjredis_delete
  delete mjredis
===============================================================================
*/
bool mjredis_delete(mjredis redis_handle) {
  if (!redis_handle) return false;
  if (redis_handle->context) redisFree(redis_handle->context);
  free(redis_handle);
  return true;
}
