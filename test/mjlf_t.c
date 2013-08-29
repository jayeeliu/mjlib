#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "mjlf_t.h"
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjproto_txt.h"
#include "mjsql.h"

static mjsql get_sql_conn(mjproto_txt_data cmd_data) {
  mjconnb conn = (mjconnb) cmd_data->conn;
  mjthread thread = mjconnb_get_obj(conn, "thread");
  mjsql sql_conn = mjthread_get_obj(thread, "sql_conn");
  return sql_conn;
}

static int is_valid_table_name(mjstr tn) {
  if (tn->length < 1 || tn->length > MJLF_MAX_TABLE_NAME_LENGTH) {
    return ERR_TABLE_NAME_LENGTH;
  }
  int i = 0;
  for (; i<tn->length; i++) {
    // allow lower and upper letter, number and _
    if (!isalnum(tn->data[i]) && tn->data[i] != 95) {
      return ERR_TABLE_NAME_ILLEGAL_CHAR;
    }
  }
  return MJLF_VALID_SUCCESS;
}

static int is_valid_key(mjstr key) {
  if (key->length < 1 || key->length > MJLF_MAX_KEY_LENGTH) {
    return ERR_KEY_LENGTH;
  }
  return MJLF_VALID_SUCCESS;
}

static int is_valid_value(mjstr value) {
  if (value->length < 1 || value->length > MJLF_MAX_VALUE_LENGTH) {
    return ERR_VALUE_LENGTH;
  }
  return MJLF_VALID_SUCCESS;
}

static void mjlf_error(int error_code, char* error_msg) {
  switch(error_code) {
    case ERR_COMMEND_FORMAT_ERROR:
      sprintf(error_msg, "%d command error. Format: command table [key] [length]\\r\\n [value\\r\\n]\r\n", error_code);
      break;
    case ERR_TABLE_NAME_LENGTH:
      sprintf(error_msg, "%d table name is too short or too long, 1~%d\r\n", error_code, MJLF_MAX_TABLE_NAME_LENGTH);
      break;
    case ERR_TABLE_NAME_ILLEGAL_CHAR:
      sprintf(error_msg, "%d table name contains illegal character(s)\r\n", error_code);
      break;
    case ERR_KEY_LENGTH:
      sprintf(error_msg, "%d the key is too short or too long, 1~%dB\r\n", error_code, MJLF_MAX_KEY_LENGTH);
      break;
    case ERR_VALUE_LENGTH:
      sprintf(error_msg, "%d the value is too short or too long, 1~%dB\r\n", error_code, MJLF_MAX_VALUE_LENGTH);
      break;
    case ERR_SQL_QUERY_FAILE:
      sprintf(error_msg, "%d execute fail\r\n", error_code);
      break;
    default:
      sprintf(error_msg, "-50000 unknow error\r\n");
      break;
  }
}


static void show_error(int error_code, mjconnb conn) {
  char error[256];
  mjlf_error(error_code, error);
  mjconnb_writes(conn, error);
}

static void show_succ(mjconnb conn, char* data) {
  if (data != NULL) {
    int len   = strlen(data);
    // 65535 + len("OK ") + 10
    char buf[65538];
    sprintf(buf, "OK %d\r\n%s", len, data);
    mjconnb_writes(conn, buf);
  } else {
    mjconnb_writes(conn, "OK\r\n");
  }
}

/*
 * validate all request params
 * @param mjstrlist args request params list
 * @param int args_legth current command arguments length, for validating request params
 * @param mjconnb conn   current connection
 */
static int mjlf_validate(mjstrlist args, unsigned int args_length, mjconnb conn) {
  int error_code;

  if (args->length != args_length) {
    show_error(ERR_COMMEND_FORMAT_ERROR, conn);
    return MJLF_VALID_FAIL;
  }

  if (args_length >= 2 && (error_code = is_valid_table_name(args->data[1])) != MJLF_VALID_SUCCESS) {
    show_error(error_code, conn);
    return MJLF_VALID_FAIL;
  }

  if (args_length >= 3 && (error_code = is_valid_key(args->data[2])) != MJLF_VALID_SUCCESS) {
    show_error(error_code, conn);
    return MJLF_VALID_FAIL;
  }

#ifdef DEBUGING
  args_length >= 2 && printf("key:%s\r\n", args->data[1]->data);
  args_length >= 3 && printf("value:%s\r\n", args->data[2]->data);
#endif

  if (args_length >= 4 && (error_code = is_valid_value(args->data[3])) != MJLF_VALID_SUCCESS) {
    show_error(error_code, conn);
    return MJLF_VALID_FAIL;
  }

  return MJLF_VALID_SUCCESS;
}

static mjstrlist filter_kv(mjstrlist args, mjsql handle, mjconnb conn) {
  if (args->length < 3) {
    return NULL;
  }

  mjstrlist query_params = mjstrlist_new();
  if (!query_params) {
    show_error(ERR_SYSTEM_ERROR, conn);
    return NULL;
  }

  if (args->length >= 3) {
    int length = args->data[2]->length * 2 + 1;
    char to[length];
    mjsql_real_escape_string(handle, to, args->data[2]->data, 129, args->data[2]->length);
    mjstrlist_adds(query_params, to);
  }

  if (args->length >= 4) {
    int length = args->data[3]->length * 2 + 1;
    char to[length];
    mjsql_real_escape_string(handle, to, args->data[2]->data, length, args->data[2]->length);
    mjstrlist_adds(query_params, to);
  }

  return query_params;
}

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

  mjstrlist_clean(query_params);
  return NULL;
}

void* PutRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 4, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[MJLF_MAX_SQL_LENGTH];
  mjsql sql_conn = get_sql_conn(cmd_data);
  mjstrlist query_params = filter_kv(args, sql_conn, cmd_data->conn);
  MJ_SET(sql_str, args->data[1]->data, args->data[2]->data, args->data[3]->data);

  if (mjsql_query(sql_conn, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAILE, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  mjstrlist_clean(query_params);
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

  if (mjsql_query(sql_conn, sql_str, strlen(sql_str)) != 0) {
    show_error(ERR_SQL_QUERY_FAILE, cmd_data->conn);
  } else {
    show_succ(cmd_data->conn, NULL);
  }

  mjstrlist_clean(query_params);
  return NULL;
}

void* CreateRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;

  if (mjlf_validate(args, 2, cmd_data->conn) != MJLF_VALID_SUCCESS) {
    return NULL;
  }

  char sql_str[1024];
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
  mjsql sql_conn = mjsql_new("172.16.139.60", "user", "passwd", "test", 3308);
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

