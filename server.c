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


#include "server_handler.h"

#define MAXLINE 4096
#define LISTENQ 10


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
