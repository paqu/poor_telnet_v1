#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

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

	// Client action
	close(sockfd);
	exit(0);
}
