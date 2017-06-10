#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>

#define MAXLINE 4096

int main(int argc, char *argv[])
{
    int ret;
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 3) {
        fprintf(stdout, "Usage %s <SERV_ADDR> <PORT>\n", argv[0]);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd < 0) {
        fprintf(stderr, "Socker error");
        return -1;
    }


    memset(&servaddr,'\0',sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(atoi(argv[2]));
    inet_pton(AF_INET,argv[1], &servaddr.sin_addr);

    ret = connect(sockfd, (struct sockaddr *) &servaddr,sizeof(servaddr));
    if (ret < 0) {
        fprintf(stderr, "Connet error");
        return -1;
    }

    struct pollfd fds[2];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    char command[MAXLINE];
    char output[MAXLINE];

    while (1) {
        ret = poll(fds, 2, -1);
        if (ret == -1 ) {
            if (errno == EINTR)
                continue;

            fprintf(stderr, "Poll error");
            return -1;
        }

        if (fds[0].revents & POLLIN) {
            printf("Pass command\n");
            memset(command, 0, MAXLINE);
            fgets(command, MAXLINE, stdin);
            write(sockfd, command, MAXLINE);
        }

        if (fds[1].revents & POLLIN) {
            memset(output, 0, MAXLINE);
            read(sockfd, output, MAXLINE);
            write(STDOUT_FILENO, output, MAXLINE);
        }
    }

    close(sockfd);
    exit(0);
}
