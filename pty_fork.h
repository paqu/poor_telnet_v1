#ifndef PTY_FORK_H
#define PTY_FORK_H
#include <fcntl.h>

pid_t ptyFork(int *masterFd);

#endif
