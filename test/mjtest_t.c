#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjstr.h"
#include "mjcomm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void* Run(void* arg) {
    mjconnb conn = (mjconnb) arg;
    mjstr data = mjstr_new(512);
    mjconnb_readuntil(conn, "\r\n\r\n", data);
    mjconnb_writes(conn, "OK\r\n");
    mjstr_delete(data);
    return NULL;
}

int main() {
    int sock = mjsock_tcp_server(7879);
    if (sock < 0) {
      printf("server socket create error");
      return 1;
    }
    mjlf server = mjlf_new(sock, Run, 20, NULL, NULL, NULL, NULL);
    if (!server) {
      printf("mjlf_new error\n");
    }
    mjlf_run(server);
    mjlf_delete(server);
    return 0;
}
