#ifndef _MJCOMM_H
#define _MJCOMM_H

#include <stdbool.h>
#include <sys/time.h>

extern int  daemonize();
extern int  process_spawn(int proc_num);

extern void save_pid_file(const char* pid_file);
extern void remove_pid_file(const char* pid_file);

extern bool set_max_open_files(int files_num);
extern bool Worker_New(char *args[], int *rfd, int *wfd);

static inline int get_cpu_count() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

static inline long long get_current_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

#endif
