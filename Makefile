CC	= gcc
CFLGS	= -std=c99 -Wall -Wextra -Werror

all: server client

server:server.o
	$(CC) $(CFLGS) -o server server.c

client:client.o
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f *.o server client
