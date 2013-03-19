#ifndef _MJCOMM_H
#define _MJCOMM_H

extern int          Daemonize();
extern long long    GetCurrentTime();
extern int          ProcessSpawn( int nProcs );
extern void         RemovePid( const char *pidFile );
extern void         SavePid( const char *pidFile );
extern int          worker_new( char *args[], int *rfd, int *wfd );

#endif