#include <stdio.h>
#include "mjsock.h"

int main()
{
    int sfd = mjSock_TcpServer(7879);

    mjSock_Close(sfd);

    return 0;
}
