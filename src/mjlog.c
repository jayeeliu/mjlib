#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include "mjlog.h"

#define MAX_MSG_LEN 512

void mjlog_init(const char* name) {
  openlog(name, LOG_CONS|LOG_PID, LOG_LOCAL6);
}

void mjlog_write(int priority, const char* message, ...) {
  char msg[MAX_MSG_LEN + 1]; 
  va_list ap; 
    
  va_start(ap, message);
  vsnprintf(msg, MAX_MSG_LEN, message, ap);
  syslog(priority, "%s", msg);
  va_end(ap);
}

void mjlog_close() {
  closelog();
}
