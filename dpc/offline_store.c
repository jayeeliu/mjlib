/*
 * DPC offline store
 *
 * command format: command table [key] [length]\r\n[value\r\n]
 * user command  : get / put / del / quit
 * admin command : create / drop / reload
 */

#include "offline_store.h"

/*
 * mysql connect
 * @TODO R/W and sharding
 */
static mjsql get_sql_conn(mjproto_txt_data cmd_data) {
  mjconnb conn = (mjconnb) cmd_data->conn;
  mjthread thread = mjconnb_get_obj(conn, "thread");
  mjsql sql_conn = mjthread_get_obj(thread, "sql_conn");
  return sql_conn;
}

/*
 * for mysql escape key and value
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
    mjsql_real_escape_string(handle, to, args->data[2]->data, 129, args->data[2]->length);
    mjstrlist_adds(query_params, to);
  }

  // value data
  if (args->length >= 5) {
    int length = args->data[3]->length * 2 + 1;
    char to[length];
    mjsql_real_escape_string(handle, to, args->data[2]->data, length, args->data[2]->length);
    mjstrlist_adds(query_params, to);
  }

  return query_params;
}

static mjstr read_value(mjconnb conn, mjstr len) {
  int length = atoi(len->data);
  mjstr content = mjstr_new(1024);
  mjconnb_readbytes(conn, content, length);
  if (length != content->length) {
    mjstr_delete(content);
    show_error(ERR_VALUE_READ_FAIL, conn);
    return NULL;
  }
  return content;
}


/*
 * command: get table key\r\n
 */
void* GetRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  mjsql sql_conn = get_sql_conn(cmd_data);
  char sql_str[MJLF_MAX_SQL_LENGTH];
  mjstrlist query_params = filter_kv(args, sql_conn, cmd_data->conn);
  MJ_GET(sql_str, args->data[1]->data, query_params->data[0]->data);
  mjstrlist_clean(query_params);

  if (mjsql_query(sql_conn, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAILE, cmd_data->conn);
    return NULL;
  }
  mjsql_store_result(sql_conn);
  if (mjsql_get_rows_num(sql_conn) == 0) {
    show_succ(cmd_data->conn, NULL);
    return NULL;
  }
  mjsql_next_row(sql_conn);
  show_succ(cmd_data->conn, mjsql_fetch_row_field(sql_conn, 0));

  return NULL;
}

/*
 * command: put table key value-length\r\nvalue-content
 */
void* PutRoutine(void* arg) {
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
  mjsql sql_conn = get_sql_conn(cmd_data);
  mjstrlist query_params = filter_kv(args, sql_conn, cmd_data->conn);
  MJ_SET(sql_str, args->data[1]->data, args->data[2]->data, args->data[4]->data);
  mjstrlist_clean(query_params);

  if (mjsql_query(sql_conn, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAILE, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}

void* DelRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 3, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  mjsql sql_conn = get_sql_conn(cmd_data);
  mjstrlist query_params = filter_kv(args, sql_conn, cmd_data->conn);
  MJ_DEL(sql_str, args->data[1]->data, args->data[2]->data);
  mjstrlist_clean(query_params);

  if (mjsql_query(sql_conn, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAILE, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}

void* CreateRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 2, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  MJ_CREATE(sql_str, args->data[1]->data);

  mjsql sql_conn = get_sql_conn(cmd_data);
  if (mjsql_query(sql_conn, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAILE, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}

void* DropRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 2, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  MJ_DROP(sql_str, args->data[1]->data);

  mjsql sql_conn = get_sql_conn(cmd_data);
  if (mjsql_query(sql_conn, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAILE, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  return NULL;
}

void* QuitRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjproto_txt_finished(cmd_data);
  show_succ(cmd_data->conn, NULL);
  return NULL;
}

struct mjproto_txt_routine_list routine_list[] = {
  {"get", GetRoutine},
  {"put", PutRoutine},
  {"del", DelRoutine},
  {"create", CreateRoutine},
  {"drop", DropRoutine},
  {"quit", QuitRoutine},
  {NULL, NULL},
};

static void* sql_quit(void* arg) {
  mjsql sql_conn = (mjsql) arg;
  mjsql_delete(sql_conn);
  return NULL;
}

static void* thread_init(void* arg) {
  mjthread thread = (mjthread) arg;
  mjsql sql_conn = mjsql_new("172.16.139.60", "test", "test", "test", 3308);
  mjthread_set_obj(thread, "sql_conn", sql_conn, sql_quit);
  return NULL;
}

int main() {
  int sfd = mjsock_tcp_server(17879);
  if (sfd < 0) {
    printf("mjsock_tcp_server error");
    return 1;
  }

  mjlf server = mjlf_new(sfd, mjproto_txt_routine, 4, mjproto_txt_init,
          routine_list, thread_init, NULL);
  if (!server) {
    printf("mjlf_New error");
    return 1;
  }
  mjlf_set_timeout(server, 10000, 10000);
  mjlf_run(server);
  mjlf_delete(server);
  return 0;
}

