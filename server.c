/****************************************************************************
 * server.c
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Implements a multi-threaded TCP server
****************************************************************************/

#include "server.h"

int main(int arg_count, char *arg_val[]) {

   // Check number of arguments
  if(4 != arg_count) {
    fprintf(stderr, "Usage: %s port num_threads max_num_connections\n", arg_val[0]);
    exit(EXIT_FAILURE);
  }

  char *port = arg_val[1];
  // int num_threads = atoi(arg_val[2]);
  // int max_num_connections = atoi(arg_val[3]);

  // Actions for connection termination handler
  struct sigaction new_action;
  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = &handle_termination;
  sigaction(SIGINT, &new_action, NULL);

  // Get list of available sockets
  struct addrinfo *socket_list = get_server_sockaddr(port);

  // Create a listening socket
  int sockfd = bind_socket(socket_list);

  // Start listening on the socket
  if(-1 == listen(sockfd, BACKLOG)) {
    err(EXIT_FAILURE, "%s", "Unable to listen on socket");
  }

  while(!term_requested) {
    // Wait for a connection and handle it
    int connectionfd = wait_for_connection(sockfd);
    handle_connection(connectionfd);
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}

// Gets a list of available sockets for listening on a specified port
struct addrinfo* get_server_sockaddr(const char* port) {
  struct addrinfo hints;    // Additional 'hints' about connection
  struct addrinfo* results; // Linked list of sockets

  // Initialize hints
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // Request IPv4 addresses
  hints.ai_socktype = SOCK_STREAM; // Request TCP sockets
  hints.ai_flags = AI_PASSIVE;     // Request a listening socket

  // Retrieve available sockets from all local IP addresses
  int retval = getaddrinfo(NULL, port, &hints, &results);
  if(retval) {
    errx(EXIT_FAILURE, "%s", gai_strerror(retval));
  }
  return results;
}

// Create and bind to a socket
int bind_socket(struct addrinfo* addr_list) {
  struct addrinfo* addr;
  int sockfd;
  char yes = '1';

  // Iterate over the addresses in the list; stop when we successfully bind to one
  for(addr = addr_list; addr != NULL; addr = addr->ai_next) {
    // Open a socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    // Move on to the next address if we couldn't open a socket
    if(-1 == sockfd) {
      continue;
    }

    // Allow the port to be re-used if currently in the TIME_WAIT state
    if(-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
      err(EXIT_FAILURE, "%s", "Unable to set socket option");
    }

    // Try to bind the socket to the address/port
    if(bind(sockfd, addr->ai_addr, addr->ai_addrlen) == -1) {
      // If binding fails, close the socket, move on to next address
      close(sockfd);
      continue;
    } else {
      // Otherwise, we've bound the address to the socket, so stop processing
      break;
    }
  }
  // Free the memory allocated to the address list
  freeaddrinfo(addr_list);

  // If addr is NULL, we tried every address and weren't able to bind to any
  if(NULL == addr) {
    err(EXIT_FAILURE, "%s", "Unable to bind to socket");
  } else {
    // Otherwise return the socket descriptor
    return sockfd;
  }
}

// Waits for a connection on a listening socket
// Returns a pointer to a connection socket
int wait_for_connection(int sockfd) {
  struct sockaddr_in client_addr;                     //Remote IP that is connecting to us
  unsigned int addr_len = sizeof(struct sockaddr_in); //Length of the remote IP structure
  char ip_address[INET_ADDRSTRLEN];                   //Buffer to store human-friendly IP
  int connectionfd;                                   //Socket file descriptor for the new conection

  // Wait for new connection
  connectionfd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);

  // Make sure connection was established
  if(-1 == connectionfd) {
    err(EXIT_FAILURE, "%s", "Unable to accept connection");
  }

  // Convert the connecting IP to a human readble form and print it
  inet_ntop(client_addr.sin_family, &client_addr.sin_addr, ip_address, sizeof(ip_address));
  printf("Connection accepted from %s\n", ip_address);

  // Return the socket file descriptor for the new connection
  return connectionfd;
}

// Handles incoming connection
void handle_connection(int connectionfd) {
  char buffer[MAX_MSG_SIZE];
  int bytes_read;

  do {
    // Read up to 1024 bytes from the client
    bytes_read = recv(connectionfd, buffer, MAX_MSG_SIZE - 1, 0);

    // If the data was read successfully
    if(bytes_read > 0) {
      // Add a terminating NULL character and print the message received
      buffer[bytes_read] = '\0';
      // printf("Message received (%d bytes): %s\n", bytes_read, buffer);

      // Calculate response (multiply by 10)
      int multiplicand = atoi(buffer);
      char *response;
      asprintf(&response, "%d", multiplicand * MULTIPLIER);

      // Echo the data back to the client; exit loop if we're unable to send
      if(-1 == send(connectionfd, response, strlen(response), 0)) {
        warn("Unable to send data to client");
        break;
      }
      free(response);
    }
  } while(bytes_read > 0 && !term_requested);

  // Close connection
  close(connectionfd);
}

// Signal handler for Ctrl+C
void handle_termination(int signal) {
  printf("\nServer shutdown requested\n");
  term_requested = true;
}