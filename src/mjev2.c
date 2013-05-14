#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "mjev2.h"
#include "mjcomm.h"
#include "mjlog.h"

/*
================================================================
mjEV2_Add 
    Add FileEvent to Loop, fd--socket handler
    mask -- MJEV_READABLE(for read) or MJEV_WRITEABLE(for write)
    proc -- event handle function, data -- args for proc
    output -1-- add event failed,  0--add event success
===============================================================
*/
bool mjEV2_Add( mjEV2 ev, int fd, int mask, mjProc Proc, void *data ) {
    // sanity check
    if ( !ev || !Proc ) {
        MJLOG_ERR( "mjev or Proc is null" );
        return false;
    }
    // fd must not large than MJEV_MAXFD
    if ( fd >= MJEV_MAXFD || fd < 0 ) {
        MJLOG_ERR( "fd is invalid: %d", fd );
        return false;
    }
    // mask check
    if ( ( mask & MJEV_READABLE ) == 0 &&
        ( mask & MJEV_WRITEABLE ) == 0 ) {
        MJLOG_ERR( "only support READ and WRITE: %d", mask );
        return false; 
    }
    // get mjfevent correspond to fd
    mjfevent2* fdev = &( ev->fileEventList[fd] );     
    int newmask = fdev->mask | mask;            // get new mask 
    // we should change epoll
    if ( fdev->mask != newmask ) {
        // set epoll event
        struct epoll_event ee;
        ee.data.u64 = 0;
        ee.data.fd  = fd;
        ee.events   = 0;
        if ( newmask & MJEV_READABLE ) ee.events |= EPOLLIN;
        if ( newmask & MJEV_WRITEABLE ) ee.events |= EPOLLOUT;
        // add event
        int op = ( fdev->mask == MJEV_NONE ) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD; 
        if ( epoll_ctl( ev->epfd, op, fd, &ee ) == -1 ) {
            MJLOG_ERR( "epoll_ctl error" );
            return false;
        }
        // set newmask
        fdev->mask = newmask;
    }
    // we must change Proc and Proc data
    if ( mask & MJEV_READABLE ) fdev->ReadCallBack = Proc;
    if ( mask & MJEV_WRITEABLE ) fdev->WriteCallBack = Proc;
    fdev->data = data;
    return true;
}

/*
================================================
mjEV2_Del
    delete event from mjev
================================================
*/
bool mjEV2_Del( mjEV2 ev, int fd, int mask ) {
    // sanity check
    if ( !ev ) {
        MJLOG_ERR( "ev is null" );
        return false;
    }
    // input parameter check 
    if ( fd >= MJEV_MAXFD || fd < 0 ) {
        MJLOG_ERR( "fd is invalid" );
        return false;
    }
    if ( ( mask & MJEV_READABLE ) == 0 &&
        ( mask & MJEV_WRITEABLE ) == 0 ) {
        MJLOG_ERR( "only support READ and WRITE: %d", mask );
        return false; 
    }
    // set mjev status
    mjfevent2 *fdev = &ev->fileEventList[fd];
    int newmask = fdev->mask & (~mask);         // get newmask
    // we should change epoll
    if ( fdev->mask != newmask ) { 
        // set epoll event
        struct epoll_event ee;
        ee.data.u64 = 0;
        ee.data.fd  = fd;
        ee.events   = 0;
        if ( newmask & MJEV_READABLE ) ee.events |= EPOLLIN;
        if ( newmask & MJEV_WRITEABLE ) ee.events |= EPOLLOUT;
        // modify or delete event
        int op = ( newmask == MJEV_NONE ) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
        if ( epoll_ctl( ev->epfd, op, fd, &ee ) == -1 ) {
            MJLOG_ERR( "epoll_ctl error" );
            return false;
        }
        // set newmask
        fdev->mask = newmask;
    }
    return true;
}

/*
========================================================================
mjEV2_AddTimer
    add timer event to mjev
========================================================================
*/
mjtevent2* mjEV2_AddTimer( mjEV2 ev, long long ms, 
        mjProc Proc, void* data ) {
    if ( !ev || !Proc ) {
        MJLOG_ERR( "ev or Proc is null" );
        return NULL;
    }
    // create mjTimerEvent struct
    mjtevent2* te = ( mjtevent2* ) calloc( 1, sizeof( mjtevent2 ) );
    if ( !te ) {
        MJLOG_ERR( "mjtevent alloc error" );
        return NULL; 
    }
    // init timerevent struct
    te->valid       = 1;                 
    te->time        = ms + GetCurrentTime();
    te->TimerProc   = Proc;
    te->data        = data;
    // insert into queue
    int ret = mjPQ_Insert( ev->timerEventQueue, te->time, te );
    if ( ret < 0 ) {
        MJLOG_ERR( "mjpq_insert error" );
        free( te );
        return NULL;
    }
    return te;
}

/*
===============================================
mjEV2_DelTimer
    invalid mjtevent
===============================================
*/
bool mjEV2_DelTimer( mjEV2 ev, mjtevent2 *te ) {
    if ( !ev || !te ) {
        MJLOG_ERR( "ev or te is null" );
        return false;
    }
    te->valid = 0;
    return true;
}

/*
===========================================================
mjEV2_Pending
    add pending proc to ev
===========================================================
*/
bool mjEV2_AddPending( mjEV2 ev, mjProc Proc, void* data ) {
    if ( !ev || !Proc ) {
        MJLOG_ERR( "ev or Proc is null" );
        return false;
    }
    if ( ev->pendingNum < PENDING_LIST_LEN ) {
        ev->pendingList[ev->pendingNum].Proc = Proc;
        ev->pendingList[ev->pendingNum].data = data;
        ev->pendingList[ev->pendingNum].next = NULL;
        ev->pendingNum++;
    } else {
        // alloc and set new pending
        mjpending2* newPending = ( mjpending2* ) calloc 
                ( 1, sizeof( mjpending2 ) );
        if ( !newPending ) {
            MJLOG_ERR( "pending struct alloc error" );
            return false;
        }
        newPending->Proc = Proc;
        newPending->data = data;
        newPending->next = NULL;
        if ( ev->pendingTail == NULL ) {
            // the first extra pending
            ev->pendingList[PENDING_LIST_LEN-1].next = newPending;
        } else {
            // not the first extra pending
            ev->pendingTail->next = newPending;
        }
        ev->pendingTail = newPending;
    }
    return true;
}

/*
=================================================
mjEV2_DelPending
    del pending proc according to data.
    it is useful. when error happends
    data is freed, before pending proc 
    called.
=================================================
*/
bool mjEV2_DelPending( mjEV2 ev, void* data ) {
    if ( !ev->pendingNum ) return true;
    for ( int i = 0; i < ev->pendingNum; i++ ) {
        if ( ev->pendingList[i].data == data ) {
            ev->pendingList[i].Proc = NULL;
            ev->pendingList[i].data = NULL;
        }
    }
    // TODO: extra list
    // TODO: pending check. when in pending proc
}

/*
==================================================
GetFirstTimer
    get first timer from timer queue
==================================================
*/
static long long GetFirstTimer( mjEV2 ev ) {
    return mjPQ_GetMinKey( ev->timerEventQueue );
}

/*
==================================================
GetFirstTimerEvent
    get first time event from queue
==================================================
*/
static mjtevent2* GetFirstTimerEvent( mjEV2 ev ) {
    mjtevent2* te = mjPQ_GetMinValue( ev->timerEventQueue );
    if ( !te ) return NULL;
    // delete timer event from queue
    mjPQ_DelMin( ev->timerEventQueue );
    return te;
}

/*
===================================
mjEV2_Run
    run event loop
===================================
*/
void mjEV2_Run( mjEV2 ev ) {
    // sanity check
    if ( !ev ) {
        MJLOG_ERR( "mjev is null" );
        return;
    }
    // epoll_wait timeout, -1 wait forver 
    long long timeWait = -1;
    // current time clock
    long long currTime;  
    // get first timer event from queue
    long long first = GetFirstTimer( ev ); 
    // we got a timer event
    if ( first != -1 ) {          
        currTime = GetCurrentTime();
        // adjust wait time, 0 return immediate
        timeWait = ( first <= currTime ) ? 0 : first - currTime;
    }
    // have pending proc run, no wait
    if ( ev->pendingNum ) timeWait = 0;
    // wait for event
    struct epoll_event epEvents[MJEV_MAXFD];
    int numevents = epoll_wait( ev->epfd, epEvents, MJEV_MAXFD, timeWait );
    if ( numevents < 0 ) {
        if ( errno == EINTR ) return;
        MJLOG_ERR( "epoll_wait error. errno: %d, msg: %s", errno, strerror( errno ) );
        return; // some error
    }
    // get timer event, run it
    first = GetFirstTimer( ev );                       
    currTime = GetCurrentTime();        
    if ( first != -1 && first <= currTime ) {           // run timer event
        mjtevent2 *te = GetFirstTimerEvent( ev );       // get timer event from queue
        if ( te && te->valid && te->TimerProc ) te->TimerProc( te->data );   // call timer proc
        free( te );                                     // free timer event, alloc in addtimer
    }
    if ( !numevents && !ev->pendingNum ) return;  // no events, break
    // get events from list
	for ( int i = 0; i < numevents; i++ ) {
        // get file event fd, and mjfevent struct
	    struct epoll_event* e = &epEvents[i];
	    int fd = e->data.fd;
	    mjfevent2* fdev = &ev->fileEventList[fd];
        // check file op mask
	    int mask = 0;
	    if ( e->events & EPOLLIN ) mask |= MJEV_READABLE;
	    if ( e->events & EPOLLOUT ) mask |= MJEV_WRITEABLE;
        // check epoll error
        if ( mask == 0 && ( e->events & EPOLLERR ) ) mask |= MJEV_READABLE | MJEV_WRITEABLE;
        // run proc according to mask
	    int rFired = 0;
	    if ( fdev->mask & mask & MJEV_READABLE ) {
            // we have run fileproc
	        rFired = 1;
		    fdev->ReadCallBack( fdev->data );
	    }
	    if ( fdev->mask & mask & MJEV_WRITEABLE ) {
		    if ( !rFired || fdev->WriteCallBack != fdev->ReadCallBack ) {
			    fdev->WriteCallBack( fdev->data );
			}
		}
	}
    if ( !ev->pendingNum ) return;
    // copy pending data
    int pendingNum = ev->pendingNum;
    mjpending2 pendingList[PENDING_LIST_LEN];
    memcpy( pendingList, ev->pendingList, sizeof( pendingList ) );
    // set pending proc to zero
    ev->pendingNum  = 0;
    ev->pendingTail = NULL;
    // run pending proc
    for ( int i = 0; i < pendingNum; i++ ) {
        // call pending proc
        if ( pendingList[i].Proc ) {
            ( pendingList[i].Proc ) ( pendingList[i].data );
        }
    }
    // run other proc
    if ( pendingNum > PENDING_LIST_LEN ) {
        mjpending2* toRun = pendingList[PENDING_LIST_LEN-1].next;
        mjpending2* Next;
        while ( toRun ) {
            Next = toRun->next;
            toRun->Proc( toRun->data ); 
            free( toRun );
            toRun = Next;
        }
    }
}

/*
=====================================================
mjEV2_New
    create mjev struct
    return NULL --- failed
=====================================================
*/
mjEV2 mjEV2_New()
{
    // create mjev struct 
    mjEV2 ev = ( mjEV2 ) calloc ( 1, sizeof( struct mjEV2 ) );
    if ( !ev ) {
        MJLOG_ERR( "mjEV2 malloc error" );
        return NULL;
    }
    // create new epoll
    ev->epfd = epoll_create( 1024 );
    if ( ev->epfd == -1 ) {
        MJLOG_ERR( "epoll_create error" );
        free( ev );
        return NULL;
    }
    // create timer event queue
    ev->timerEventQueue = mjPQ_New();
    if ( !ev->timerEventQueue ) {
        MJLOG_ERR( "mjpq alloc error" );
        close( ev->epfd );
        free( ev );
        return NULL;
    }
    return ev;
}

/*
======================================================
mjEV2_ReleasePending
    release Pending struct
======================================================
*/
static void mjEV2_ReleasePending( mjEV2 ev ) {
    if ( ev->pendingNum <= PENDING_LIST_LEN ) return;
    // release pending struct 
    mjpending2* toRelease = ev->pendingList[PENDING_LIST_LEN-1].next;
    mjpending2* Next;
    while ( toRelease ) {
        Next = toRelease->next;
        free( toRelease );
        toRelease = Next;
    }
}

/*
==============================
mjEV2_Delete
    Delete mjev object. 
    No Return.
==============================
*/
bool mjEV2_Delete( mjEV2 ev ) {
    // sanity check
    if ( !ev ) {
        MJLOG_ERR( "mjev is null" );
        return false;
    }
    close( ev->epfd );
    mjPQ_Delete( ev->timerEventQueue );
    // release pending struct
    mjEV2_ReleasePending( ev );
    free( ev );
    return true;
}
