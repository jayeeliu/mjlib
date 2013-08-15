#ifndef _MJSIG_H
#define _MJSIG_H

#include <signal.h>

typedef void sighandler(int sig);

void mjsig_init();
void mjsig_register(int sig, sighandler handler);
void mjsig_process_queue();

#endif
