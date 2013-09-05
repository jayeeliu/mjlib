/*
 * DPC online store
 *
 * command format: command table [key] [length]\r\n[value\r\n]
 * user command  : get / put / del / quit
 * admin command : create / drop / reload
 */

#include "online_store.h"


void* online_thread_init(void* arg) {
  return NULL;
}

static void* online_connection_clean(void* arg) {/*{{{*/
  mjredis conn = (mjredis) arg;
  mjredis_delete(conn);
  return NULL;
}/*}}}*/

/* mysql connect
 * {{{
 * @param mode online_MODE_WRITE / online_MODE_READ
 * @TODO sharding
 */
static mjredis get_connection(mjconnb conn, char* mode, char* table, char* key) {
  char table_name[35];
  sprintf(table_name, "%s##%s", mode, table);

  mjthread thread = mjconnb_get_obj(conn, "thread");
  mjredis handle = mjthread_get_obj(thread, table_name);
  if (!handle) {
    struct online_dsn dsn = {};
    mjopt_get_value_string(table, "master", dsn.host);
    /*
    if (mode == DPC_MODE_WRITE) {
      mjopt_get_value_string(table, "master", dsn.host);
    } else {
      mjopt_get_value_string(table, "slave", dsn.host);
    }
    */
    mjopt_get_value_int(table, "port", &dsn.port);
    mjopt_get_value_string(table, "database", dsn.database);

    if (!dsn.host || !dsn.database || !dsn.port) {
      MJLOG_ERR("online config not find or config error in %s", table);
      show_error(ERR_SERVER_CONFIG, conn);
      return NULL;
    }

    mjredis handle = mjredis_new(dsn.host, dsn.port);

    if (!handle) {
      MJLOG_ERR("redis connect fail %s:%d@db%d", dsn.host, dsn.port, dsn.database);
      show_error(ERR_STORE_CONNECT_FAIL, conn);
      return NULL;
    }

    if (dsn.database) {
      mjredis_select(handle, dsn.database);
    }

    mjthread_set_obj(thread, table_name, handle, online_connection_clean);
  } else {
    // avoid connect error for the same host:port but database not
    char db[12];
    mjopt_get_value_string(table, "database", db);
    mjredis_select(handle, db);
  }

  return handle;
}/* }}} */

/* command: get table key\r\n
 * {{{
 */
void* online_get(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjredis handle = get_connection(cmd_data->conn, DPC_MODE_READ,
      args->data[1]->data, args->data[2]->data);

  mjstr ret = mjstr_new(128);
  if (mjredis_get(handle, args->data[2]->data, ret) <= REDIS_EXEC_FAIL_CODE) {
    show_error(ERR_REDIS_EXEC_FAIL, cmd_data->conn);
    mjstr_delete(ret);
    return NULL;
  }
  show_succ(cmd_data->conn, ret->data);
  mjstr_delete(ret);

  return NULL;
}/*}}}*/

/* command: put table key value-length\r\nvalue-content
 * {{{
 */
void* online_put(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 4, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjstr value = read_value(cmd_data->conn, args->data[3]);
  if (!value) {
    return NULL;
  }

  mjredis handle = get_connection(cmd_data->conn, DPC_MODE_WRITE,
      args->data[1]->data, args->data[2]->data);

  if (mjredis_set(handle, args->data[2]->data, value->data) <= REDIS_EXEC_FAIL_CODE) {
    show_error(ERR_REDIS_EXEC_FAIL, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }
  mjstr_delete(value);

  return NULL;
}/*}}}*/

/* command: del table key
 * {{{
 */
void* online_del(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjredis handle = get_connection(cmd_data->conn, DPC_MODE_WRITE,
      args->data[1]->data, args->data[2]->data);

  if (mjredis_del(handle, args->data[2]->data) <= REDIS_EXEC_FAIL_CODE) {
    show_error(ERR_REDIS_EXEC_FAIL, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}/*}}}*/

/* command: rpop table key\r\n
 * {{{
 */
void* online_rpop(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjredis handle = get_connection(cmd_data->conn, DPC_MODE_READ,
      args->data[1]->data, args->data[2]->data);

  mjstr ret = mjstr_new(128);
  if (mjredis_rpop(handle, args->data[2]->data, ret) <= REDIS_EXEC_FAIL_CODE) {
    show_error(ERR_REDIS_EXEC_FAIL, cmd_data->conn);
    mjstr_delete(ret);
    return NULL;
  }
  show_succ(cmd_data->conn, ret->data);
  mjstr_delete(ret);

  return NULL;
}/*}}}*/

/* command: lpush table key value-length\r\nvalue-content
 * {{{
 */
void* online_lpush(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 4, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjstr value = read_value(cmd_data->conn, args->data[3]);
  if (!value) {
    return NULL;
  }

  mjredis handle = get_connection(cmd_data->conn, DPC_MODE_WRITE,
      args->data[1]->data, args->data[2]->data);

  int code = mjredis_lpush(handle, args->data[2]->data, value->data);
  if (code <= REDIS_EXEC_FAIL_CODE) {
    show_error(ERR_REDIS_EXEC_FAIL, cmd_data->conn);
  } else {
    char ret[12];
    sprintf(ret, "%d", code);
    show_succ(cmd_data->conn, ret);
  }
  mjstr_delete(value);

  return NULL;
}/*}}}*/

/* command: llen table key
 * {{{
 */
void* online_llen(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjstr value = read_value(cmd_data->conn, args->data[3]);
  if (!value) {
    return NULL;
  }

  mjredis handle = get_connection(cmd_data->conn, DPC_MODE_WRITE,
      args->data[1]->data, args->data[2]->data);

  int code = mjredis_llen(handle, args->data[2]->data);
  if (code <= REDIS_EXEC_FAIL_CODE) {
    show_error(ERR_REDIS_EXEC_FAIL, cmd_data->conn);
  } else {
    char ret[12];
    sprintf(ret, "%d", code);
    show_succ(cmd_data->conn, ret);
  }
  mjstr_delete(value);

  return NULL;
}/*}}}*/

/* command: create table
 * {{{
 */
void* online_create(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  show_error(ERR_COMMAND_NOT_SUPPORT_NOW, cmd_data->conn);
  return NULL;
}/*}}}*/

/* command: drop table
 * {{{
 */
void* online_drop(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  show_error(ERR_COMMAND_NOT_SUPPORT_NOW, cmd_data->conn);
  return NULL;
}/*}}}*/

/* command: quit
 * {{{
 */
void* online_quit(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjproto_txt_finished(cmd_data);
  show_succ(cmd_data->conn, NULL);
  return NULL;
}/*}}}*/

