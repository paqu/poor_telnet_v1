#include "server_handler.h"
#include "pty_fork.h"

#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

int server_handler(int fd1,int fd2) {
    int fdm;
    int ret;
    char input[MAXLINE];

    pid_t childPid;

    childPid = ptyFork(&fdm);
    if (childPid == -1) {
        fprintf(stderr, "ptyFork error\n");
        _exit(EXIT_FAILURE);
    }

    if (childPid == 0) {
        char *child_av[2] = {"sh", NULL};
        execvp(child_av[0], child_av);
        fprintf(stderr, "execvp error\n");
        _exit(EXIT_FAILURE);
    }

    fd_set fd_in;

    while (1) {
        FD_ZERO(&fd_in);
        FD_SET(fd1, &fd_in);
        FD_SET(fdm, &fd_in);

        ret = select(MAX(fdm, fd1) + 1, &fd_in, NULL, NULL, NULL);
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            exit(1);
        }

        if (FD_ISSET(fd1, &fd_in)) {
            ret = read(fd1, input, sizeof(input));

            if (ret > 0) {
                input[ret] ='\0';
                write(fdm, input, ret);
            }else if (ret == 0) {
                    fprintf(stdout, "Client disconnected");
                    exit(1);
            }else {
                fprintf(stderr, "Socket read error");
                _exit(EXIT_FAILURE);
            }
        }

        if (FD_ISSET(fdm, &fd_in)) {
            ret = read(fdm, input, sizeof(input));
            if (ret > 0) {
                input[ret] ='\0';
                write(fd2, input, ret);
            }else {
                if (ret <= 0) {
                    exit(1);
                }
            }
         }
    }
    return 0;
}
