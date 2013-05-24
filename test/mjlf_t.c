#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjcomm.h"
#include "mjopt.h"
#include "mjproto_txt.h"

struct server_data {
  char id[1024];
};

void* GetRoutine(void* arg)
{
  struct mjProtoTxtData* cmdData = 
        (struct mjProtoTxtData*) arg;
  mjConnB_WriteS(cmdData->conn, "Get Called\r\n");
  return NULL; 
} 

void* PutRoutine(void* arg)
{
  struct mjProtoTxtData* cmdData = 
        (struct mjProtoTxtData*) arg;
  mjConnB_WriteS(cmdData->conn, "Put Called\r\n");
  return NULL;
}

void* StatRoutine(void* arg)
{
  struct mjProtoTxtData* cmdData = 
        (struct mjProtoTxtData*) arg;
  mjConnB_WriteS(cmdData->conn, "OK Here\r\n");
  return NULL;
}

void* TestRoutine(void* arg)
{
  struct mjProtoTxtData* cmdData = 
        (struct mjProtoTxtData*) arg;
  mjConnB conn = cmdData->conn;
  mjLF  srv  = conn->server;
  struct server_data* s_data = srv->private;
  mjConnB_WriteS(cmdData->conn, s_data->id);
  return NULL;
}

PROTO_TXT_ROUTINE routineList[] = {
  {"get", GetRoutine},
  {"put", PutRoutine},
  {"test", TestRoutine},
  {"stat", StatRoutine},
};

void* Routine(void* arg)
{
  mjConnB conn = (mjConnB) arg;
  mjTxt_RunCmd(routineList, 
    sizeof(routineList) / sizeof(PROTO_TXT_ROUTINE), conn);
  mjConnB_Delete(conn);
  return NULL;
}


int main()
{
  int port;
  int threadNum;

  mjOpt_Define(NULL, "port", MJOPT_INT, &port, "7879");
  mjOpt_Define(NULL, "threadnum", MJOPT_INT, &threadNum, "20");
  mjOpt_ParseConf("test.conf");

  int sfd = mjSock_TcpServer(port);
  if (sfd < 0) {
    printf("mjSock_TcpServer error");
    return 1;
  }

  struct server_data s_data;
  strcpy(s_data.id, "1013\r\n");
  
  mjLF server = mjLF_New(sfd, Routine, threadNum);
  if (!server) {
    printf("mjLF_New error");
    return 1;
  }
  mjLF_SetTimeout(server, 3000, 3000);
  mjLF_SetPrivate(server, &s_data, NULL);
  mjLF_Run(server);
  mjLF_Delete(server);
  return 0;
}
