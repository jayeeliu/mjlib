#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "mjcomm.h"

int main()
{
    char *arg[] = { "./ptest" };
    int rfd, wfd;

    int ret = worker_new(arg, &rfd, &wfd);
    if (ret < 0) {
        printf("worker new error\n");
        return 1;
    }

    char buf[1024] = {0};

    strcpy(buf, "this is a test");
    write(wfd, buf, strlen(buf));

    memset(buf, 0, sizeof(buf));

    read(rfd, buf, 1024);

    printf("%s\n", buf);

    int status;
    waitpid(-1, &status, 0);
    
    return 0;
}
