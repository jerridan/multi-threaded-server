/****************************************************************************
 * client.c
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Implements a simple client that connects to a server over TCP
****************************************************************************/

#include "client.h"

int main(int arg_count, char *arg_val[]) {

  // Check number of arguments
  if(4 != arg_count) {
    fprintf(stderr, "Usage: %s server_hostname port number_to_multiply\n", arg_val[0]);
    exit(EXIT_FAILURE);
  }

  char *server_hostname = arg_val[1];
  char *port = arg_val[2];
  char *multiplicand = arg_val[3];
  char response[MAX_MSG_SIZE]; // Response from server

  // Connect to the server
  struct addrinfo* results = get_sockaddr(server_hostname, port);
  int sockfd = open_connection(results);

  // Send the message
  if(-1 == send(sockfd, multiplicand, strlen(multiplicand), 0)) {
    err(EXIT_FAILURE, "%s", "Unable to send");
  }

  // Get the server's response
  int bytes_received;
  bytes_received = recv(sockfd, &response, MAX_MSG_SIZE, 0);
  if(-1 == bytes_received) {
    err(EXIT_FAILURE, "%s", "Unable to read");
  }
  response[bytes_received] = '\0';

  printf("Result from server: %d\n", atoi(response));

  // Close the connection
  close(sockfd);

  exit(EXIT_SUCCESS);
}


// Returns a list of socket addresses available for use, given a hostname
// and port
struct addrinfo* get_sockaddr(const char* hostname, const char* port) {
  struct addrinfo hints;    // Additional 'hints' about connection
  struct addrinfo* results; // Linked list of sockets

  // Initialize hints, request IPv4 and TCP
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM; // Return TCP results

  // Retrieve available sockets
  // If successful, retval will = 0
  int retval = getaddrinfo(hostname, port, &hints, &results);
  if(retval) {
    // Convert error to human-readable string, log, and exit
    errx(EXIT_FAILURE, "%s", gai_strerror(retval));
  }
  return results;
}

// Returns a pointer to a socket connection, given a list of available
// sockets, or -1 if connection cannot be opened
int open_connection(struct addrinfo* addr_list) {
  struct addrinfo* addr; // Current socket address
  int sockfd;            // Pointer to open socket

  for(addr = addr_list; addr != NULL; addr = addr->ai_next) {
    // Open a socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    // If unsuccessful, try the next address
    if(-1 == sockfd) {
      continue;
    }

    // Stop iterating if we're able to connect to the server
    if(-1 != connect(sockfd, addr->ai_addr, addr->ai_addrlen)) {
      break;
    }
  }
  // Free memory allocated to the addrinfo list
  freeaddrinfo(addr_list);

  // Log error and exit if connection failed
  if(NULL == addr) {
    err(EXIT_FAILURE, "%s", "Unable to connect to server");
  } else {
    return sockfd;
  }
}