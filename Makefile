CC	= gcc
CFLGS	= -std=c99 -Wall -Wextra -Werror

all: server

server:server.o
	$(CC) $(CFLGS) -o server server.c

clean:
	rm -f *.o server
