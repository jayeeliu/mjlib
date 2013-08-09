#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjstr.h"
#include "mjhttpreq.h"

void* Run(void* arg) {
    mjconnb conn = (mjconnb) arg;
    mjStr data = mjStr_New();
    mjconnb_ReadUntil(conn, "\r\n\r\n", data);
    mjconnb_WriteS(conn, "OK");
    mjStr_Delete(data);
    mjconnb_Delete(conn);
    return NULL;
}

int main() {
    int sock = mjSock_TcpServer(7879);
    mjLF server = mjLF_New(sock, Run, 20);
    mjLF_Run(server);
    mjLF_Delete(server);
    return 0;
}
