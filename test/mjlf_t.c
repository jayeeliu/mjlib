#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjopt.h"
#include "mjproto_txt.h"

void* GetRoutine(void* arg) {
  struct mjproto_txt_data* cmdData = (struct mjproto_txt_data*) arg;
  mjconnb_writes(cmdData->conn, "Get Called\r\n");
  return NULL; 
} 

void* PutRoutine(void* arg) {
  struct mjproto_txt_data* cmdData = (struct mjproto_txt_data*) arg;
  mjconnb_writes(cmdData->conn, "Put Called\r\n");
  return NULL;
}

void* StatRoutine(void* arg) {
  struct mjproto_txt_data* cmdData = (struct mjproto_txt_data*) arg;
  mjconnb_writes(cmdData->conn, "OK Here\r\n");
  return NULL;
}

void* QuitRoutine(void* arg) {
  struct mjproto_txt_data* cmdData = (struct mjproto_txt_data*) arg;
  mjconnb conn = cmdData->conn;
  mjconnb_writes(cmdData->conn, "Quit\r\n");
  conn->_closed = true;
  return NULL;
}

PROTO_TXT_ROUTINE routineList[] = {
  {"get", GetRoutine},
  {"put", PutRoutine},
  {"stat", StatRoutine},
  {"quit", QuitRoutine},
};

void* Routine(void* arg) {
  mjconnb conn = (mjconnb) arg;
  while (!conn->_closed && !conn->_timeout) {
    mjtxt_run_cmd(routineList, 
				sizeof(routineList) / sizeof(PROTO_TXT_ROUTINE), conn);
  }
  return NULL;
}

int main() {
  int sfd = mjsock_tcp_server(7879);
  if (sfd < 0) {
    printf("mjSock_TcpServer error");
    return 1;
  }

  mjlf server = mjlf_new(sfd, Routine, 4, NULL, NULL, NULL, NULL);
  if (!server) {
    printf("mjlf_New error");
    return 1;
  }
  mjlf_set_timeout(server, 3000, 3000);
  mjlf_run(server);
  mjlf_delete(server);
  return 0;
}
