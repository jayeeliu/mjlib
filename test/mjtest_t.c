#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjstr.h"
#include "mjcomm.h"
#include "mjthread.h"
#include <stdio.h>
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
  free(thread->thread_local);
  return NULL;
}

void* Run(void* arg) {
    mjconnb conn = (mjconnb) arg;
    mjStr data = mjStr_New();
    mjconnb_readuntil(conn, "\r\n\r\n", data);
    mjconnb_writes(conn, conn->shared);
    mjStr_Delete(data);
    return NULL;
}

int main() {
    int sock = mjsock_tcp_server(7879);
    if (sock < 0) {
      printf("server socket create error");
      return 1;
    }
    mjlf server = mjlf_new(sock, Run, get_cpu_count(), Init, NULL, Exit);
    mjlf_run(server);
    mjlf_delete(server);
    return 0;
}
