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

  const char *port = arg_val[1];
  const int num_threads = atoi(arg_val[2]);
  const int max_num_connections = atoi(arg_val[3]);

  // Check for valid entries
  if(num_threads < 1) {
    fprintf(stderr, "The number of threads must be a postive integer\n");
    exit(EXIT_FAILURE);
  }
  
  if(max_num_connections < 1) {
    fprintf(stderr, "The number of connections must be a postive integer\n");
    exit(EXIT_FAILURE);
  }

  // Actions for connection termination handler
  struct sigaction new_action;
  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = &handle_termination;
  sigaction(SIGINT, &new_action, NULL);

  // Allocate space for connections queue
  connections = calloc(max_num_connections, sizeof(int));
  num_connections = 0; // # connections currently in queue

  // Initialize queue mutex
  if(pthread_mutex_init(&queue_mutex, NULL)) {
    err(EXIT_FAILURE, "%s", "Unable to initialize queue mutex");
  }

  // Create threads that will handle client requests
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  for(int i = 0; i < num_threads; i++) {
    if(pthread_create(&threads[i], NULL, thread_handle_connection, NULL)) {
      err(EXIT_FAILURE, "%s", "Error while creating thread");
    }
  }

  // Get list of available sockets
  struct addrinfo *socket_list = get_server_sockaddr(port);

  // Create a listening socket
  int sockfd = bind_socket(socket_list);

  // Start listening on the socket
  if(-1 == listen(sockfd, BACKLOG)) {
    err(EXIT_FAILURE, "%s", "Unable to listen on socket");
  }

  // Initialize barrier - for testing only
  // pthread_barrier_init(&barrier, NULL, num_threads);

  int connectionfd; // Incoming connection socket

  while(!term_requested) {
    // Wait for a connection from a client
    connectionfd = wait_for_connection(sockfd);
    if(-1 == connectionfd) {
      break;
    }

    // If queue is full, drop the connection
    if(max_num_connections == num_connections) {
      close(connectionfd);

      // Otherwise put the connection in queue
    } else {
      if(pthread_mutex_lock(&queue_mutex)) {
        err(EXIT_FAILURE, "%s", "Unable to lock queue mutex, aborting");
      }

      connections[num_connections++] = connectionfd;
      
      if(pthread_mutex_unlock(&queue_mutex)) {
        err(EXIT_FAILURE, "%s", "Unable to unlock queue mutex, aborting");
      }
    }
  }

  close(sockfd);
  printf("Server shutdown successful\n");
  exit(EXIT_SUCCESS);
}

// Thread main function for handling connections
void *thread_handle_connection(void *arg) {
  char buffer[MAX_MSG_SIZE]; // Receive buffer
  int bytes_read;

  do {

    // pthread_barrier_wait(&barrier); // Barrier for threads - for testing only

    // If there aren't any connections, sleep and recheck every second
    while(!num_connections && !term_requested) {
      sleep(1);
    }

    // Lock out connections queue and grab the first one
    pthread_mutex_lock(&queue_mutex);
    int connectionfd = remove_connection_from_queue();
    pthread_mutex_unlock(&queue_mutex);

    if(-1 == connectionfd) {
      continue;
    }

    // Read up to 1024 bytes from the client
    bytes_read = recv(connectionfd, buffer, MAX_MSG_SIZE - 1, 0);

    // If the data was read successfully
    if(bytes_read > 0) {
      // Add a terminating NULL character and print the message received
      buffer[bytes_read] = '\0';

      // Calculate response
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

    // Close connection
    close(connectionfd);

  } while(bytes_read > 0 && !term_requested);

  return NULL;
}

// Removes the first connection from the queue and returns it
int remove_connection_from_queue() {
  // If there are no more connections (another thread grabbed the last one)
  if(!num_connections) {
    return -1;
  }
  int connectionfd = connections[0];
  for(int i = 0; i < num_connections - 1; i++) {
    connections[i] = connections[i+1];
  }
  num_connections--;
  return connectionfd;
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
    fprintf(stderr, "Unable to accept connection\n");
    return -1;
  }

  // Convert the connecting IP to a human readble form and print it
  inet_ntop(client_addr.sin_family, &client_addr.sin_addr, ip_address, sizeof(ip_address));
  printf("Connection accepted from %s\n", ip_address);

  // Return the socket file descriptor for the new connection
  return connectionfd;
}

// Signal handler for Ctrl+C
void handle_termination(int signal) {
  printf("\nServer shutdown requested\n");
  // pthread_barrier_destroy(&barrier); // Remove barrier - for testing only
  term_requested = true;
}