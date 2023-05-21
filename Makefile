CC = gcc
CFLAGS = -Wall -Wextra -g

all: client server

client: client.o
	$(CC) $(CFLAGS) -o clientT client.o

server: server.o
	$(CC) $(CFLAGS) -o serverR server.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f *.o clientT serverR
