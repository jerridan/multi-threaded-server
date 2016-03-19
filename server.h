/****************************************************************************
 * server.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Implements a multi-threaded TCP server
****************************************************************************/

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#define BACKLOG 25       // Backlog maximum for queued connections to a socket
#define MAX_MSG_SIZE 256 // Maximum size of msgs from client
#define MULTIPLIER 10    // Number to multiply requests by

static bool term_requested = false; // True when SIGINT caught

int main();

// Gets a list of available sockets for listening on a specified port
struct addrinfo* get_server_sockaddr(const char* port);

// Create and bind to a socket
int bind_socket(struct addrinfo* addr_list);

// Waits for a connection on a listening socket
// Returns a pointer to a connection socket
int wait_for_connection(int sockfd);

// Handles incoming connection
void handle_connection(int connectionfd);

// Signal handler for Ctrl+C
void handle_termination(int signal);
