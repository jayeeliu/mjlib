#ifndef _MJCOMM_H
#define _MJCOMM_H

#include <stdbool.h>

extern int        Daemonize();
extern long long  GetCurrentTime();
extern int        ProcessSpawn(int nProcs);
extern void       RemovePid(const char *pidFile);
extern void       SavePid(const char *pidFile);
extern int        get_cpu_count();
extern bool				set_max_open_files(int files_num);
extern bool       Worker_New(char *args[], int *rfd, int *wfd);

#endif
