#ifndef __MJWORKER_H
#define __MJWORKER_H

typedef void workproc( void* arg );

extern int WorkerRun( int minProcs, int maxProcs, int sfd, workproc* proc );

#endif
