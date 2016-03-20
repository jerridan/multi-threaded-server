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
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define BACKLOG 25       // Backlog maximum for queued connections to a socket
#define MAX_MSG_SIZE 256 // Maximum size of msgs from client
#define MULTIPLIER 10    // Number to multiply requests by

static int *connections;            // Array of connections threads can handle
static int num_connections = 0;     // # connections currently in queue
static bool term_requested = false; // True when SIGINT caught
pthread_mutex_t queue_mutex;        // Connection queue mutex

// pthread_barrier_t barrier; // Barrier for testing multi-threading

int main();

// Thread main function for handling connections
void *thread_handle_connection(void *arg);

// Removes the first connection from the queue and returns it
int remove_connection_from_queue();

// Gets a list of available sockets for listening on a specified port
struct addrinfo* get_server_sockaddr(const char* port);

// Create and bind to a socket
int bind_socket(struct addrinfo* addr_list);

// Waits for a connection on a listening socket
// Returns a pointer to a connection socket
int wait_for_connection(int sockfd);

// Signal handler for Ctrl+C
void handle_termination(int signal);
