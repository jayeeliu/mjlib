#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "mjev.h"
#include "mjcomm.h"
#include "mjlog.h"

/*
================================================================
mjEV_Add 
    Add FileEvent to Loop, fd--socket handler
    mask -- MJEV_READABLE(for read) or MJEV_WRITEABLE(for write)
    proc -- event handle function, data -- args for proc
    output -1-- add event failed,  0--add event success
===============================================================
*/
bool mjEV_Add( mjEV ev, int fd, int mask, mjProc Proc, void *data ) {
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
    mjfevent* fdev = &( ev->fileEventList[fd] );     
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
mjEV_Del
    delete event from mjev
================================================
*/
bool mjEV_Del( mjEV ev, int fd, int mask ) {
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
    mjfevent *fdev = &ev->fileEventList[fd];
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
mjEV_AddTimer
    add timer event to mjev
========================================================================
*/
mjtevent* mjEV_AddTimer( mjEV ev, long long ms, 
        mjProc Proc, void* data ) {
    if ( !ev || !Proc ) {
        MJLOG_ERR( "ev or Proc is null" );
        return NULL;
    }
    // create mjTimerEvent struct
    mjtevent* te = ( mjtevent* ) calloc( 1, sizeof( mjtevent ) );
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
mjEV_DelTimer
    invalid mjtevent
===============================================
*/
bool mjEV_DelTimer( mjEV ev, mjtevent *te ) {
    if ( !ev || !te ) {
        MJLOG_ERR( "ev or te is null" );
        return false;
    }
    te->valid = 0;
    return true;
}

/*
===========================================================
mjEV_Pending
    add pending proc to ev
===========================================================
*/
bool mjEV_AddPending( mjEV ev, mjProc Proc, void* data ) {
    if ( !ev || !Proc ) {
        MJLOG_ERR( "ev or Proc is null" );
        return false;
    }
    // alloc and set new pending
    mjpending* newPending = ( mjpending* ) calloc 
                ( 1, sizeof( mjpending ) );
    if ( !newPending ) {
        MJLOG_ERR( "pending struct alloc error" );
        return false;
    }
    newPending->Proc = Proc;
    newPending->data = data;
    INIT_LIST_HEAD( &newPending->pendingNode );
    list_add_tail( &newPending->pendingNode, &ev->pendingHead ); 
    return true;
}

/*
=================================================
mjEV_DelPending
    del pending proc according to data.
    it is useful. when error happends
    data is freed, before pending proc 
    called.
=================================================
*/
bool mjEV_DelPending( mjEV ev, void* data ) {
    if ( list_empty( &ev->pendingHead ) ) return false;
    mjpending* entry;
    mjpending* tmp;
    list_for_each_entry_safe( entry, tmp, &ev->pendingHead, pendingNode ) {
        if ( entry->data == data ) {
            list_del( &entry->pendingNode );
            free( entry );
        }
    }
    return true; 
}

/*
==================================================
GetFirstTimer
    get first timer from timer queue
==================================================
*/
static long long GetFirstTimer( mjEV ev ) {
    return mjPQ_GetMinKey( ev->timerEventQueue );
}

/*
==================================================
GetFirstTimerEvent
    get first time event from queue
==================================================
*/
static mjtevent* GetFirstTimerEvent( mjEV ev ) {
    mjtevent* te = mjPQ_GetMinValue( ev->timerEventQueue );
    if ( !te ) return NULL;
    // delete timer event from queue
    mjPQ_DelMin( ev->timerEventQueue );
    return te;
}

/*
===================================
mjEV_Run
    run event loop
===================================
*/
void mjEV_Run( mjEV ev ) {
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
    if ( !list_empty( &ev->pendingHead ) ) timeWait = 0;
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
        mjtevent *te = GetFirstTimerEvent( ev );       // get timer event from queue
        if ( te && te->valid && te->TimerProc ) te->TimerProc( te->data );   // call timer proc
        free( te );                                     // free timer event, alloc in addtimer
    }
    if ( !numevents && list_empty( &ev->pendingHead ) ) return;  // no events, break
    // get events from list
	for ( int i = 0; i < numevents; i++ ) {
        // get file event fd, and mjfevent struct
	    struct epoll_event* e = &epEvents[i];
	    int fd = e->data.fd;
	    mjfevent* fdev = &ev->fileEventList[fd];
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
    // check and run pending proc
    if ( list_empty( &ev->pendingHead ) ) return;
    struct list_head savedPendingHead;
    INIT_LIST_HEAD( &savedPendingHead );
    list_splice_init( &ev->pendingHead, &savedPendingHead );
    // run pending proc
    mjpending* entry;
    mjpending* tmp;
    list_for_each_entry_safe( entry, tmp, &savedPendingHead, pendingNode ) {
        list_del( &entry->pendingNode );
        entry->Proc( entry->data );
        free( entry );
    }
}

/*
=====================================================
mjEV_New
    create mjev struct
    return NULL --- failed
=====================================================
*/
mjEV mjEV_New() {
    // create mjev struct 
    mjEV ev = ( mjEV ) calloc ( 1, sizeof( struct mjEV ) );
    if ( !ev ) {
        MJLOG_ERR( "mjEV malloc error" );
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
    INIT_LIST_HEAD( &ev->pendingHead );
    return ev;
}

/*
======================================================
mjEV_ReleasePending
    release Pending struct
======================================================
*/
static void mjEV_ReleasePending( mjEV ev ) {
    if ( list_empty( &ev->pendingHead ) ) return;
    // release pending struct 
    mjpending* entry;
    mjpending* tmp;
    list_for_each_entry_safe( entry, tmp, &ev->pendingHead, pendingNode ) {
        list_del( &entry->pendingNode );
        free( entry );
    }
}

/*
==============================
mjEV_Delete
    Delete mjev object. 
    No Return.
==============================
*/
bool mjEV_Delete( mjEV ev ) {
    // sanity check
    if ( !ev ) {
        MJLOG_ERR( "mjev is null" );
        return false;
    }
    close( ev->epfd );
    mjPQ_Delete( ev->timerEventQueue );
    // release pending struct
    mjEV_ReleasePending( ev );
    free( ev );
    return true;
}
