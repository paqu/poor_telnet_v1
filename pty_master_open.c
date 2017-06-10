#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "pty_master_open.h"

int ptyMasterOpen(char *slaveName, size_t snLen)
{
    int masterFd, err, ret;
    char *pts_name;

    /* Open pty master */
    masterFd =  posix_openpt(O_RDWR | O_NOCTTY);
    if (masterFd == -1)
        return -1;

    /* grant access to the slave psueudoterminal */
    ret = grantpt(masterFd);
    if (ret == -1) {
        err = errno;
        close(masterFd);
        errno = err;
        return -1;
    }
    /* unlock a pseudoterminal master/slave pair */
    ret = unlockpt(masterFd);
    if (ret == -1) {
        err = errno;
        close(masterFd);
        errno = err;
        return -1;
    }

    /* get the name of the slave pseudoterminal */
    pts_name = ptsname(masterFd);
    if (pts_name == NULL) {
        err = errno;
        close(masterFd);
        errno = err;
        return -1;
    }

    if (strlen(pts_name) < snLen) {
        strncpy(slaveName, pts_name, snLen);
    } else {
        close(masterFd);
        errno = EOVERFLOW;
        return -1;
    }

    return masterFd;
}

