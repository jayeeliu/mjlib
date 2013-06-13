#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mjcomm.h"
#include "mjlog.h"
#include "mjsock.h"

/*
===========================================================
Daemonize
    daemonize the process. Parent exit.
    return:     0   --- success
                -1  --- fail
===========================================================
*/
int Daemonize() {
    switch(fork()) {
	case -1:
	    return -1;
    case 0:
	    break;
    default:
	    _exit(EXIT_SUCCESS);
	}

	if (setsid() == -1) {
        MJLOG_ERR("setsid error");
        return -1;
    }

	if (chdir("/") != 0) {
        MJLOG_ERR("chdir error");
		return -1;
	}

    int fd;
	if ((fd = open("/dev/null", O_RDWR, 0)) == -1) {
        MJLOG_ERR("open /dev/null error");
		return -1;
	}

	if (dup2(fd, STDIN_FILENO) < 0) {
        MJLOG_ERR("dup2 STDIN_FILENO error");
		return -1;
	}

	if (dup2(fd, STDOUT_FILENO) < 0) {
        MJLOG_ERR("dup2 STDOUT_FILENO error");
		return -1;
	}

	if (dup2(fd, STDERR_FILENO) < 0) {
        MJLOG_ERR("dup2 STDERR_FILENO error");
		return -1;
	}

	if (fd > STDERR_FILENO) {
		close(fd);
	}

	return 0;
}

/*
================================================
GetCurrentTime
    get current time
================================================
*/
long long GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int ProcessSpawn(int nProcs)
{
    Daemonize();
    for(int i = 0; i < nProcs; i++) {
        pid_t pid = fork();
        // child return
        if (!pid) return 0;
        // error return
        if (pid < 0) {
            MJLOG_ERR("frok error");
            return -1;
        }
    }
    // parent run here
    exit(0);
    return 1;
}

/*
==============================================
SavePid
    save pid to file.
==============================================
*/
void SavePid(const char* pidFile)
{
	if (!pidFile) {
        MJLOG_ERR("pidFile is null");
        return;
    }

	FILE *fp;
	if (!(fp = fopen(pidFile, "w"))) {
        MJLOG_ERR("open pid file %s error", pidFile);
		return;
	}

	fprintf(fp, "%ld\n", (long)getpid());

	if (fclose(fp) == -1) {
        MJLOG_ERR("close pid file %s error", pidFile);
        return;
	}
}

/*
=================================================
RemovePid
    remove pid file.
=================================================
*/
void RemovePid(const char* pidFile)
{
	if (!pidFile) {
        MJLOG_ERR ("pidFile is null");
        return;
    }

	if (unlink(pidFile)) {
        MJLOG_ERR ("unlike pid file %s error", pidFile);
	}
}

/*
====================================
GetCPUNumer
    get online cpu number
    return: number of online cpus 
====================================
 */
int GetCPUNumber()
{
    int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    return cpu_num;
}

/*
===============================================================================
Worker_New
    Run worker
    args: worker filename
    rfd: read handler
    wfd: write handler
    return: true or false
===============================================================================
*/
bool Worker_New(char* args[], int* rfd, int* wfd) {
    int pin[2], pout[2];
    if (pipe(pin) == -1) goto failout1;
    if (pipe(pout) == -1) goto failout2;

    pid_t pid = vfork();
    if (pid < 0) goto failout3;
    // child run, never return
    if (pid == 0) {
        close(pin[0]);
        close(pout[1]); 
        dup2(pout[0], 0);
        dup2(pin[1], 1);
        execv(*args, args);
        exit(0);
    }
    // parent run
    *rfd = pin[0];
    *wfd = pout[1];

    close(pin[1]);
    close(pout[0]);
    return true;

failout3:
    close(pout[0]);
    close(pout[1]);
failout2:
    close(pin[0]);
    close(pin[1]);
failout1:
    return false;
}
