#include <string.h>
#include <signal.h>
#include "mjlog.h"
#include "mjsig.h"

#define MAX_SIGNAL 256

// signal struct
struct signal_descriptor {
  int         count;
  sighandler* handler;
};

static int  signal_queue_len;
static int  signal_queue[MAX_SIGNAL];
static struct signal_descriptor signal_state[MAX_SIGNAL];
static sigset_t blocked_sig;

/*
===============================================================================
mjSig_Init
    init signal
===============================================================================
*/
void mjsig_init() {
  signal_queue_len = 0;
  memset(signal_queue, 0, sizeof(signal_queue));
  memset(signal_state, 0, sizeof(signal_state));
  sigfillset(&blocked_sig);
}

/*
===============================================================================
signal_handler
	default signal handler proc
===============================================================================
*/
static void signal_handler(int sig) {
	// not handler this signal, ignore
  if (sig < 0 || sig > MAX_SIGNAL || !signal_state[sig].handler) {
    MJLOG_ERR("Received unhandled signal %d. Signal has been disabled.", sig);
    signal(sig, SIG_IGN);
    return;
  }
	// put signal in queue
  if (!signal_state[sig].count) {
    if (signal_queue_len < MAX_SIGNAL) {
      signal_queue[signal_queue_len++] = sig;
    } else {
      MJLOG_ERR("Signal %d: signal queue is unexpected full.", sig);
    }
  }
	// add signal count
  signal_state[sig].count++;
  signal(sig, signal_handler);
}

/*
===============================================================================
mjsig_register
	register signal handler
===============================================================================
*/
void mjsig_register(int sig, sighandler handler) {
	// sanity check
	if (sig < 0 || sig > MAX_SIGNAL) return;
	// set signal state
	signal_state[sig].count = 0;
 	if (handler == NULL) handler = SIG_IGN; 
	// set signal handler
	if (handler != SIG_IGN && handler != SIG_DFL) {
		signal_state[sig].handler = handler;
 		signal(sig, signal_handler);
 	} else {                        
		// set default handler
   	signal_state[sig].handler = NULL;
 		signal(sig, handler);
	}
}

/*
===============================================================================
mjsig_process_queue
	mjsig process queue
===============================================================================
*/
void mjsig_process_queue() {
	// block all signal delivery
	sigset_t old_sig;
	sigprocmask(SIG_SETMASK, &blocked_sig, &old_sig);
 	// check signal queue	
	for (int cur_pos = 0; cur_pos < signal_queue_len; cur_pos ++) {
		int sig = signal_queue[cur_pos];
 		struct signal_descriptor *desc = &signal_state[sig];
		if (desc->count) {
   		if (desc->handler) desc->handler(sig);
   		desc->count = 0;
 		}
 	}
	signal_queue_len = 0;
 	// restore signal delivery
	sigprocmask(SIG_SETMASK, &old_sig, NULL);  
}
