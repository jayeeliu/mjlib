#ifndef _MJSIG_H
#define _MJSIG_H

#include <signal.h>

typedef void sighandler( int sig );

void mjSig_Init();
void mjSig_Register( int sig, sighandler handler );
void mjSig_ProcessQueue();

#endif
