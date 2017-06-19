CC	= gcc
CFLGS	= -std=c99 -Wall -Wextra -Werror

all: server client

server:server.o pty_fork.o pty_master_open.o server_handler.o
	$(CC) -o server server.o pty_fork.o pty_master_open.o server_handler.o


server.o:server.c
	$(CC) -c server.c -o $@

pty_fork.o: pty_fork.c
	$(CC) $(CFLGS) -c pty_fork.c -o $@

pty_master_open.o: pty_master_open.c
	$(CC) $(CFLGS) -c pty_master_open.c -o $@

client:client.o
	$(CC) $(CFLAGS) -o client client.c

server_handler.o: server_handler.c
	$(CC) $(CFLGS) -c server_handler.c -o $@

clean:
	rm -f *.o server client
