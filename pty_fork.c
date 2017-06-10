#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pty_master_open.h"
#include "pty_fork.h"


#define MAX_SNAME 1000

pid_t ptyFork(int *masterFd)
{
    int mfd, slaveFd, err, ret;
    pid_t childPid;
    char slname[MAX_SNAME];

    mfd = ptyMasterOpen(slname, MAX_SNAME);
    if (mfd == -1)
        return -1;

    childPid = fork();

    if (childPid == -1) {
        err = errno;
        close(mfd);
        errno = err;
        return -1;
    }

    /*parent*/
    if (childPid != 0) {
        *masterFd = mfd;
        return childPid;
    }

    ret = setsid();
    if (ret == -1) {
        fprintf(stderr, "setsid() on error");
        _exit(EXIT_FAILURE);
    }

    close(mfd);

    slaveFd = open(slname, O_RDWR);
    if (slaveFd == -1) {
        fprintf(stderr, "Slave terminal open error");
        _exit(EXIT_FAILURE);
    }

#ifdef TIOSCTTY
    if (ioctl(slaveFd, TIOSCTTY, 0) == -1) {
        fprintf(stderr, "ioctl error");
        _exit(EXIT_FAILURE);
    }
#endif

    if (dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO) {
        fprintf(stderr,"stdin dup2 error");
        _exit(EXIT_FAILURE);
    }

    if (dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO) {
        fprintf(stderr,"stdout dup2 error");
        _exit(EXIT_FAILURE);
    }

    if (dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO) {
        fprintf(stderr,"stderr dup2 error");
        _exit(EXIT_FAILURE);
    }

    if (slaveFd > STDERR_FILENO)
        close(slaveFd);

    return 0;
}

