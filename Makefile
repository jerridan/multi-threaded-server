CC         = gcc
CFLAGS     = -g -O0 -Wall -Werror -std=gnu11
LDFLAGS    = -L.
SERVERLIBS = -lpthread

.PHONY: all clean

all: client server

client: client.o
	$(CC) -o $@ $^ $(LDFLAGS)

client.o: client.c client.h
	$(CC) $(CFLAGS) -o $@ -c $<

server: server.o
	$(CC) -o $@ $^ $(LDFLAGS) $(SERVERLIBS)

server.o: server.c server.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a client server
