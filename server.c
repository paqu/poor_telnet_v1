#define _XOPEN_SOURCE 600
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

#include <termios.h>
#include <sys/ioctl.h>


#define MAXLINE 4096
#define MAX(a,b) (((a)>(b))?(a):(b))
#define LISTENQ 10

void sig_chld(int signo);
int server_handler(int fd1,int fd2);

int main(int argc, char *argv[])
{
    int ret;
    int listenfd, connfd;
    char address[MAXLINE];

    socklen_t clilen;
    pid_t childpid;

    struct sockaddr_in cliaddr, servaddr;

    if (argc != 2) {
        fprintf(stdout, "Usage: %s <PORT>\n", argv[0]);
        return -1;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        fprintf(stderr, "Socket error");
        return -1;
    }

    memset(&servaddr,'\0',sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(listenfd,(struct sockaddr *) &servaddr, sizeof(servaddr));
    if (ret < 0) {
        fprintf(stderr, "Bind error");
        return -1;
    }


    ret = listen(listenfd,LISTENQ);
    if (ret < 0) {
        fprintf(stderr, "listen error");
        return -1;
    }

    fprintf(stdout, "Server is running ... \n");

    signal(SIGCHLD,sig_chld);

    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd,
            (struct sockaddr *) &cliaddr, &clilen)) < 0 ) {
            if (errno == EINTR)
                continue;
            else {
                fprintf(stderr, "Accept error");
                return 0;
            }
        }
        fprintf(stdout, "Connection from %s, port %d \n",
                inet_ntop(AF_INET,&cliaddr.sin_addr,address,sizeof(address)),
                ntohs(cliaddr.sin_port));

        if ( 0 == (childpid = fork())) {
            close(listenfd);
            server_handler(connfd, connfd);
            exit(0);
        }
        close(connfd);
    }

    return 0;
}

int server_handler(int fd1,int fd2) {
    int fdm, fds;
    int ret, err;
    char * pts_name;
    char input[MAXLINE];

    /* open pseudoterminal as master */
    fdm = posix_openpt(O_RDWR);
    if (fdm == -1) {
        err = errno;
        fprintf(stderr, "Error %d on posix_openpt()\n", errno);
        return err;
    }
    /* grant access to the slave psueudoterminal */
    ret = grantpt(fdm);
    if (ret == -1) {
        err = errno;
        fprintf(stderr, "Error %d on grantpt()\n", errno);
        return err;
    }
    /* unlock a pseudoterminal master/slave pair */
    ret = unlockpt(fdm);
    if (ret == -1) {
        err = errno;
        fprintf(stderr, "Error %d on unlockpt()\n", errno);
        return err;
    }

    /* get the name of the slave pseudoterminal */
    pts_name = ptsname(fdm);
    if (pts_name == NULL) {
        fprintf(stderr, "Error on ptsname()\n");
        return EXIT_FAILURE;
    }

    fds = open(pts_name, O_RDWR);
    if (fds == -1) {
        err = errno;
        fprintf(stderr, "Error on open");
        return err;
    }

    if (fork()) {
        fd_set fd_in;
        close(fds);

        while (1) {
            FD_ZERO(&fd_in);
            FD_SET(fd1, &fd_in);
            FD_SET(fdm, &fd_in);

            ret = select(MAX(fdm, fd1) + 1, &fd_in, NULL, NULL, NULL);
            switch(ret) {
            case -1 :
                if (errno == EINTR)
                    continue;
                exit(1);
            default : {
                if (FD_ISSET(fd1, &fd_in)) {
                    ret = read(fd1, input, sizeof(input));
                    write(1,input, ret);

                    if (ret > 0) {
                        input[ret] ='\0';
                        write(fdm, input, ret);
                    }else {
                        if (ret < 0) {
                            exit(1);
                        }
                    }
                }
                if (FD_ISSET(fdm, &fd_in)) {
                    ret = read(fdm, input, sizeof(input));
                    write(1,input, ret);
                    if (ret > 0) {
                        input[ret] ='\0';
                        write(fd2, input, ret);
                    }else {
                        if (ret < 0) {
                            exit(1);
                        }
                    }
                 }
              }
           }
        }
    }else {

        close(fdm);

        close(0);
        close(1);
        close(2);

        dup(fds);
        dup(fds);
        dup(fds);

        close(fds);

        setsid();

        ioctl(0, TIOCSCTTY, 1);

        {
                char *child_av[2] = {
                "sh",
                NULL
                };

                ret = execvp(child_av[0], child_av);
        }

        return 1;
    }

}

void sig_chld(int signo)
{
    (void) signo;
    pid_t pid;
    int stat;

    while ( (pid = waitpid(-1,&stat,WNOHANG)) > 0)
        printf("Client process [%d] terminated",pid);

    return;
}

