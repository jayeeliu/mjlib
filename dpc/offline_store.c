/*
 * DPC offline store
 *
 * command format: command table [key] [length]\r\n[value\r\n]
 * user command  : get / put / del / quit
 * admin command : create / drop / reload
 * @TODO LOG
 */

#include "offline_store.h"

void* offline_thread_init(void* arg) {
  return NULL;
}

static void* offline_clean(void* arg) {/*{{{*/
  mjsql handle = (mjsql) arg;
  mjsql_delete(handle);
  return NULL;
}/*}}}*/

/* mysql connect
 * {{{
 * @param mode OFFLINE_MODE_WRITE / OFFLINE_MODE_READ
 * @TODO sharding
 */
static mjsql get_connection(mjconnb conn, const char* mode,
    const char* tn, const char* key) {
  char table[32];
  strcpy(table, tn);
  char table_name[35];
  sprintf(table_name, "%s##%s", mode, table);

  mjthread thread = mjconnb_get_obj(conn, "thread");
  mjsql handle = mjthread_get_obj(thread, table_name);
  if (!handle) {
    struct offline_dsn dsn = {};
    if (mode == DPC_MODE_WRITE) {
      mjopt_get_value_string(table, "master", dsn.host);
      mjopt_get_value_string(table, "user_w", dsn.user);
      mjopt_get_value_string(table, "password_w", dsn.password);
    } else {
      mjopt_get_value_string(table, "slave", dsn.host);
      mjopt_get_value_string(table, "user_r", dsn.user);
      mjopt_get_value_string(table, "password_r", dsn.password);
    }
    mjopt_get_value_int(table, "port", &dsn.port);
    mjopt_get_value_string(table, "database", dsn.database);

    if (!dsn.host || !dsn.user || !dsn.password || !dsn.database || !dsn.port) {
      MJLOG_ERR("config not find or config error in %s", table);
      show_error(ERR_SERVER_CONFIG, conn);
      return NULL;
    }

    handle = mjsql_new(dsn.host, dsn.user, dsn.password, dsn.database, dsn.port);
    if (!handle) {
      MJLOG_ERR("mysql connect fail %s@%s:%d@%s", dsn.user, dsn.host,
          dsn.port, dsn.database);
      show_error(ERR_STORE_CONNECT_FAIL, conn);
      return NULL;
    }
    mjthread_set_obj(thread, table_name, handle, offline_clean);
  }

  return handle;
}/* }}} */

/* for mysql escape key and value
 * {{{
 */
static mjstrlist filter_kv(mjstrlist args, mjsql handle, mjconnb conn) {
  if (args->length < 3) {
    return NULL;
  }

  mjstrlist query_params = mjstrlist_new();
  if (!query_params) {
    show_error(ERR_SYSTEM_ERROR, conn);
    return NULL;
  }

  // key
  if (args->length >= 3) {
    int length = args->data[2]->length * 2 + 1;
    char to[length];
    mjsql_real_escape_string(handle, to, args->data[2]->data, length, args->data[2]->length);
    mjstrlist_adds(query_params, to);
  }

  // value data
  if (args->length >= 5) {
    int length = args->data[4]->length * 2 + 1;
    char to[length];
    mjsql_real_escape_string(handle, to, args->data[4]->data, length, args->data[4]->length);
    mjstrlist_adds(query_params, to);
  }

  return query_params;
}/*}}}*/


/* command: get table key\r\n
 * {{{
 */
void* offline_get(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjsql handle = get_connection(cmd_data->conn, DPC_MODE_READ,
      args->data[1]->data, args->data[2]->data);
  if (!handle) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  mjstrlist query_params = filter_kv(args, handle, cmd_data->conn);
  MJ_GET(sql_str, args->data[1]->data, query_params->data[0]->data);
  mjstrlist_delete(query_params);

  if (mjsql_query(handle, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAIL, cmd_data->conn);
    return NULL;
  }
  mjsql_store_result(handle);
  if (mjsql_get_rows_num(handle) == 0) {
    show_succ(cmd_data->conn, NULL);
    return NULL;
  }
  mjsql_next_row(handle);
  show_succ(cmd_data->conn, mjsql_fetch_row_field(handle, 0));

  return NULL;
}/*}}}*/

/* command: put table key value-length\r\nvalue-content
 * {{{
 */
void* offline_put(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 4, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjstr value = read_value(cmd_data->conn, args->data[3]);
  if (!value) {
    return NULL;
  }

  mjstrlist_add(args, value);
  mjstr_delete(value);

  char sql_str[MJLF_MAX_SQL_LENGTH];
  mjsql handle = get_connection(cmd_data->conn, DPC_MODE_WRITE,
      args->data[1]->data, args->data[2]->data);
  if (!handle) {
    return NULL;
  }

  mjstrlist query_params = filter_kv(args, handle, cmd_data->conn);
  MJ_SET(sql_str, args->data[1]->data, query_params->data[0]->data,
      query_params->data[1]->data);
  mjstrlist_delete(query_params);

  if (mjsql_query(handle, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAIL, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}/*}}}*/

/* command: del table key
 * {{{
 */
void* offline_del(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  mjsql handle = get_connection(cmd_data->conn, DPC_MODE_WRITE,
      args->data[1]->data, args->data[2]->data);
  if (!handle) {
    return NULL;
  }

  mjstrlist query_params = filter_kv(args, handle, cmd_data->conn);
  MJ_DEL(sql_str, args->data[1]->data, query_params->data[0]->data);
  mjstrlist_delete(query_params);

  if (mjsql_query(handle, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAIL, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}/*}}}*/

/* command: create table
 * {{{
 */
void* offline_create(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 2, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  MJ_CREATE(sql_str, args->data[1]->data);

  mjsql handle = get_connection(cmd_data->conn, DPC_MODE_WRITE,
      args->data[1]->data, NULL);
  if (!handle) {
    return NULL;
  }

  if (mjsql_query(handle, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAIL, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}/*}}}*/

/* command: drop table
 * {{{
 */
void* offline_drop(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 2, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  MJ_DROP(sql_str, args->data[1]->data);

  mjsql handle = get_connection(cmd_data->conn, DPC_MODE_WRITE, args->data[1]->data, NULL);
  if (!handle) {
    return NULL;
  }

  int errno = 0;
  // skip table not exist error
  if ((errno = mjsql_query(handle, sql_str, strlen(sql_str))) != 0 && errno != 1051) {
    show_error(ERR_SQL_QUERY_FAIL, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}/*}}}*/

/* command: quit
 * {{{
 */
void* offline_quit(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjproto_txt_finished(cmd_data);
  show_succ(cmd_data->conn, NULL);
  return NULL;
}/*}}}*/

