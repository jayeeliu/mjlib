#include "mjlf.h"
#include "mjsock.h"
#include "mjconnb.h"
#include "mjstr.h"
#include "mjhttpreq.h"

void* Run(void* arg) {
    mjConnB conn = (mjConnB) arg;
    mjStr data = mjStr_New();
    mjConnB_ReadUntil(conn, "\r\n\r\n", data);
    mjHttpReq req = mjHttpReq_New(data);
    mjConnB_Write(conn, req->location);
    mjHttpReq_Delete(req);
    mjStr_Delete(data);
    mjConnB_Delete(conn);
    return NULL;
}

int main() {
    int sock = mjSock_TcpServer(7879);
    mjLF server = mjLF_New(sock, Run, 10);
    mjLF_Run(server);
    mjLF_Delete(server);
    return 0;
}
