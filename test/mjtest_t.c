#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjstr.h"
#include "mjcomm.h"
#include "mjthread.h"
#include "mjsql.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void* Init(void* arg) {
  char* buf = (char*) malloc(sizeof(char)*100);
  strcpy(buf, "Test String");
  return buf;
  
}

void* Exit(void* arg) {
  mjthread thread = (mjthread) arg;
  free(thread->local);
  return NULL;
}

void* Run(void* arg) {
    mjconnb conn = (mjconnb) arg;
    mjStr data = mjStr_New();
    mjconnb_readuntil(conn, "\r\n\r\n", data);
    mjconnb_writes(conn, conn->shared);
    mjStr_Delete(data);
    mjconnb_delete(conn);
    return NULL;
}

int main() {
    int sock = mjSock_TcpServer(7879);
    mjlf server = mjlf_new(sock, Run, get_cpu_count() * 2, Init, Exit);
    mjlf_run(server);
    mjlf_delete(server);
    return 0;
}
