/****************************************************************************
 * client.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Implements a simple client that connects to a server over TCP
****************************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_MSG_SIZE 256 // Maximum size of msgs from server

int main();

// Returns a list of socket addresses available for use, given a hostname
// and port
struct addrinfo* get_sockaddr(const char* hostname, const char* port);

// Returns a pointer to a socket connection, given a list of available
// sockets, or -1 if connection cannot be opened
int open_connection(struct addrinfo* addr_list);

#endif
