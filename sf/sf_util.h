#ifndef __SF_UTIL_H
#define __SF_UTIL_H

#include <unistd.h>
#include <sys/time.h>

static inline unsigned long 
get_current_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static inline unsigned
get_cpu_count() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

#endif
