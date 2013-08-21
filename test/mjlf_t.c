#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjproto_txt.h"

void* GetRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
  mjconnb_writes(cmd_data->conn, "Get Called\r\n");
  return NULL; 
} 

void* PutRoutine(void* arg) {
  mjproto_txt_data cmd_data = (mjproto_txt_data) arg;
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

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("mjSock_TcpServer error");
    return 1;
  }

  mjlf server = mjlf_new(sfd, mjproto_txt_routine, 4, mjproto_txt_init,
			routine_list, NULL, NULL);
  if (!server) {
    printf("mjlf_New error");
    return 1;
  }
  mjlf_set_timeout(server, 3000, 3000);
  mjlf_run(server);
  mjlf_delete(server);
  return 0;
}
