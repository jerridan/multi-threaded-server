README for Client and Multi-Threaded Server

Compiling:
Enter 'make' and both the client and server will be compiled.

Client:
To run the client, enter './client', followed by the 3 required parameters:
1. Server's hostname
2. Port number
3. Number to multiply (integers only)
Example: ./client localhost 5000 -3

Server:
To run the server, enter './server', followed by the 3 required parameters:
1. Port number
2. Number of threads to create
3. The maximum number of queued connections
Example: ./server 5000 10 20

To shut down the server, enter 'Ctrl+C'

