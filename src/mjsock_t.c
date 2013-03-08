#include <stdio.h>
#include "mjsock.h"

int main()
{
    int sfd = mjsock_tcpserver(7879);

    mjsock_close(sfd);

    return 0;
}
