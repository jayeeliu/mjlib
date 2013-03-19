#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "mjsig.h"

void term_handler(int sig)
{
    printf("signal caught: %d\n", sig);
    
    return;
}

int main()
{   
    mjSig_Init();
    
    mjSig_Register(SIGTERM, term_handler);
    mjSig_Register(SIGINT, term_handler);

    while(1) {
        mjSig_ProcessQueue();
        sleep(1);
    }

    return 0;
}
