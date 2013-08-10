#ifndef _MJLOG_H
#define _MJLOG_H

#include <syslog.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#define MJLOG_EMERG(fmt, arg...)     mjlog_write(LOG_EMERG|LOG_LOCAL6, "[%s:%d][%d][%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg)
#define MJLOG_ALERT(fmt, arg...)     mjlog_write(LOG_ALERT|LOG_LOCAL6, "[%s:%d][%d][%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg)
#define MJLOG_CRIT(fmt, arg...)      mjlog_write(LOG_CRIT|LOG_LOCAL6, "[%s:%d][%d][%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg)
#define MJLOG_ERR(fmt, arg...)       mjlog_write(LOG_ERR|LOG_LOCAL6, "[%s:%d][%d][%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg)
#define MJLOG_WARNING(fmt, arg...)   mjlog_write(LOG_WARNING|LOG_LOCAL6, "[%s:%d][%d][%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg)
#define MJLOG_NOTICE(fmt, arg...)    mjlog_write(LOG_NOTICE|LOG_LOCAL6, "[%s:%d][[%d]%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg)
#define MJLOG_INFO(fmt, arg...)      mjlog_write(LOG_INFO|LOG_LOCAL6, "[%s:%d][%d][%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg)
#define MJLOG_DEBUG(fmt, arg...)     mjlog_write(LOG_DEBUG|LOG_LOCAL6, "[%s:%d][%d][%x]: "#fmt, __FILE__, __LINE__, getpid(), pthread_self(), ##arg) 

extern void mjlog_init(const char* name);
extern void mjlog_write(int priority, const char* message, ...);
extern void mjlog_close();

#endif
