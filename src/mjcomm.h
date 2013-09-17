#ifndef _MJCOMM_H
#define _MJCOMM_H

#include <stdbool.h>

extern int        daemonize();
extern long long  get_current_time();
extern int        process_spawn(int proc_num);

extern void       save_pid_file(const char* pid_file);
extern void       remove_pid_file(const char* pid_file);

extern int        get_cpu_count();
extern bool       set_max_open_files(int files_num);
extern bool       Worker_New(char *args[], int *rfd, int *wfd);

#endif
