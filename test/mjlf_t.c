#include <stdio.h> 
#include <string.h>
#include <unistd.h>
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

void* GetRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;
  if (args->length != 2) {
    mjconnb_writes(cmd_data->conn, "+ get command error\r\n");
    return NULL;
  }

  mjsql sql_conn = get_sql_conn(cmd_data);
  char sql_str[1024];
  sprintf(sql_str, "select value from kv where key_name=\"%s\"", args->data[1]->data);
  mjsql_query(sql_conn, sql_str, strlen(sql_str));
  mjsql_store_result(sql_conn);
  if (mjsql_get_rows_num(sql_conn) == 0) {
    mjconnb_writes(cmd_data->conn, "+ no value found\r\n");
    return NULL;
  }

  while (mjsql_next_row(sql_conn)) {
    mjconnb_writes(cmd_data->conn, mjsql_fetch_row_field(sql_conn, 0));
    mjconnb_writes(cmd_data->conn, "\r\n");
  }
  return NULL; 
} 

void* PutRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjstrlist args = cmd_data->args;
  if (args->length != 3) {
    mjconnb_writes(cmd_data->conn, "+ put command error\r\n");
    return NULL;
  }
  mjconnb_writes(cmd_data->conn, "Put Called\r\n");
  return NULL;
}

void* StatRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjconnb_writes(cmd_data->conn, "OK Here\r\n");
  return NULL;
}

void* QuitRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjconnb_writes(cmd_data->conn, "Quit\r\n");
	mjproto_txt_finished(cmd_data);
  return NULL;
}

struct mjproto_txt_routine_list routine_list[] = {
  {"get", GetRoutine},
  {"put", PutRoutine},
  {"stat", StatRoutine},
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
  mjsql sql_conn = mjsql_new("127.0.0.1", "root", "", "test", 3306);
  mjthread_set_obj(thread, "sql_conn", sql_conn, sql_quit);
  return NULL;
}

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("mjSock_TcpServer error");
    return 1;
  }

  mjlf server = mjlf_new(sfd, mjproto_txt_routine, 4, mjproto_txt_init,
			routine_list, thread_init, NULL);
  if (!server) {
    printf("mjlf_New error");
    return 1;
  }
  mjlf_set_timeout(server, 3000, 3000);
  mjlf_run(server);
  mjlf_delete(server);
  return 0;
}
