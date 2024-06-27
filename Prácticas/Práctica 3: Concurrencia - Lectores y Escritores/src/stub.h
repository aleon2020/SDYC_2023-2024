#ifndef STUB_H
#define STUB_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <getopt.h>

#define MAX_THREADS 600

/*client_threads{} structure:
  mode: Indicates the operating mode (read/write).
  thread_id: Indicates the thread identifier.*/
struct client_threads {
    char *mode;
    int thread_id;  
};

/*operations{} enumeration:
  WRITE: The client notifies that he wants to write.
  READ: The client notifies that it wants to read.*/
enum operations {
    WRITE = 0,
    READ
};

/*request{} structure:
  action: Contains values ​​from the operations enumeration (WRITE/READ).
  id: Defines the id of the client making the request.*/
struct request {
    enum operations action;
    unsigned int id;
};

/*response{} structure:
  action: Contains values ​​from the operations enumeration (WRITE/READ).
  counter: Receives the value of the counter.
  latency_time: Receives the number of nanoseconds that a
  client to access the critical region of the server.*/
struct response {
    enum operations action;
    unsigned int counter;
    long latency_time;
};

// SERVER FUNCTIONS AND STRUCTURES
void initialize_server(char *ip, int port, char *priority);
void initialize_counter(int counter);
void initialize_client_thread(int *thread_parameters);
struct request receive_client_request(int client_socket);
struct response process_client_request(struct request request);
void initialize_thread_count(int threads_counter);
void *process_client_communication(void *pointer);

// CLIENT FUNCTIONS AND STRUCTURES
void initialize_network_parameters(char name[6], char *ip, unsigned int port);
struct response receive_server_response(int server_socket);
void *process_server_communication(void *pointer);

#endif