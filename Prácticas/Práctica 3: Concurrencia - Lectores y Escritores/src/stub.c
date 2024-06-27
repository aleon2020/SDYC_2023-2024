#include "stub.h"

pthread_mutex_t global_mutex, threads_mutex, fd_mutex;
pthread_mutex_t counter_mutex, writers_mutex, readers_mutex;
pthread_cond_t full_cond_var, writers_cond_var, readers_cond_var;
struct sockaddr_in server_address, client_address;
char process_name[6];
int threads[MAX_THREADS];
int counter_value = 0, num_readers = 0, num_writers = 0, num_threads = 0;
int priority_mode, tcp_socket;

/*initialize_server() function:
  Start the server by configuring a socket, assignment of an IP address and 
  port, plus priority of the same.*/
void initialize_server(char* ip, int port, char* priority) {
    setbuf(stdout, NULL);
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        fprintf(stderr, "error while creating socket\n");
        exit(EXIT_FAILURE); 
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);
    if (bind(tcp_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "error while binding socket\n");
        close(tcp_socket);
        exit(EXIT_FAILURE); 
    }
    if (listen(tcp_socket, 1000) < 0) {
        fprintf(stderr, "error while listening to the socket\n");
        close(tcp_socket);
        exit(EXIT_FAILURE);        
    }
    if (strcmp(priority, "reader") == 0) {
        priority_mode = READ;
    } else {
        priority_mode = WRITE;
    }
}

/*initialize_counter() function:
  Assigns the given value to the global variable counter.*/
void initialize_counter(int counter) {
    counter_value = counter;
}

/*initialize_client_thread() function:
  Accept a new connection from the client, manage the number of active threads
  and stores the client socket in an array, in addition to updating the thread
  information.*/
void initialize_client_thread(int *thread_parameters) {
    socklen_t address_size = sizeof(client_address);
    int client_socket = accept(tcp_socket, (struct sockaddr*)&client_address, &address_size);
    if (client_socket < 0) {
        fprintf(stderr, "error while accepting socket\n");
        close(client_socket);
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&fd_mutex);
    if (num_threads >= MAX_THREADS) {
        pthread_cond_wait(&full_cond_var, &fd_mutex);
    }
    int id = 0;
    while (id < MAX_THREADS) {
        if (threads[id] == 0) {
            threads[id] = 1;
            break;
        }
        id++;
    }
    pthread_mutex_unlock(&fd_mutex);
    pthread_mutex_lock(&fd_mutex);
    threads[id] = client_socket;
    pthread_mutex_unlock(&fd_mutex);
    thread_parameters[0] = id;
    thread_parameters[1] = client_socket;
}

/*receive_client_request{} structure:
  Receive a request structure from the client socket and returns it.*/
struct request receive_client_request(int client_socket) {
    struct request received_request;
    if (recv(client_socket, (void *)&received_request, sizeof(received_request), 0) < 0) {
        fprintf(stderr, "error while sending data to the socket\n");
        exit(EXIT_FAILURE);
    }  
    return received_request;
};

/*process_client_request{} structure:
  Processes the received request, updates the counter, handles synchronization
  between threads and returns as a response the action performed, the counter 
  value and the latency time.*/
struct response process_client_request(struct request request) {
    int blocked = 0;
    char *action, *mode_verb;
    struct response response;
    struct timespec waiting_time, initial_waiting_time, final_waiting_time;
    waiting_time.tv_sec = 0;
    waiting_time.tv_nsec = rand() % 75000000 + 75000000;
    if (clock_gettime(CLOCK_MONOTONIC, &initial_waiting_time) != 0) {
        fprintf(stderr, "error while getting time from the clock\n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&counter_mutex);
    if (request.action == READ) {
        num_readers++;
    } else {
        num_writers++;
    }
    if (request.action == READ && (num_writers > 1 || num_readers > 1)) {
        pthread_mutex_lock(&readers_mutex);
        pthread_mutex_unlock(&counter_mutex);
        pthread_cond_wait(&readers_cond_var, &readers_mutex);
        pthread_mutex_unlock(&readers_mutex);    
    }
    else if (request.action == WRITE && (num_writers > 1 || num_readers > 1)) {
        pthread_mutex_lock(&writers_mutex);
        pthread_mutex_unlock(&counter_mutex);
        pthread_cond_wait(&writers_cond_var, &writers_mutex);
        pthread_mutex_unlock(&writers_mutex);         
    } else {
        pthread_mutex_unlock(&counter_mutex);
    }
    if ((num_writers > 0 && request.action == READ) || (request.action == WRITE)) {
        pthread_mutex_lock(&global_mutex);
        blocked = 1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &final_waiting_time) != 0) {
        fprintf(stderr, "error while getting time from the clock\n");
        exit(EXIT_FAILURE);
    }
    if (request.action == WRITE) {
        counter_value++;
        action = "ESCRITOR";
        mode_verb = "modifica";
    } else {
        action = "LECTOR";
        mode_verb = "lee";
    }
    if (nanosleep(&waiting_time, NULL) < 0) {
        fprintf(stderr, "error while sleeping time from the clock\n");
        pthread_mutex_unlock(&global_mutex);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "[%ld] [%s #%d] %s contador con valor %d\n", waiting_time.tv_nsec/1000, action, request.id, mode_verb, counter_value);
    FILE *file = fopen("server_output.txt", "w");
    fprintf(file, "%d", counter_value);
    fclose(file);
    pthread_mutex_lock(&counter_mutex);
    if (request.action == READ) {
        num_readers--;
    } else {
        num_writers--;
    }
    if (priority_mode == READ && num_readers > 0) {
        pthread_cond_signal(&readers_cond_var);
    } else if (priority_mode == WRITE && num_writers > 0) {
        pthread_cond_signal(&writers_cond_var);
    } else if (request.action == READ && num_readers > 0) {
        pthread_cond_signal(&readers_cond_var);
    } else if (request.action == WRITE && num_writers > 0) {
        pthread_cond_signal(&writers_cond_var);
    } else if (request.action == READ && num_readers == 0) {
        pthread_cond_signal(&writers_cond_var);
    } else if (request.action == WRITE && num_writers == 0) {
        pthread_cond_signal(&readers_cond_var);
    } else {
        pthread_cond_signal(&readers_cond_var);
    }
    pthread_mutex_unlock(&counter_mutex);
    if (blocked) {
        pthread_mutex_unlock(&global_mutex);
    }
    response.action = request.action;
    response.counter = counter_value;
    long waiting_time_sec = final_waiting_time.tv_sec - initial_waiting_time.tv_sec;
    long waiting_time_nsec = final_waiting_time.tv_nsec - initial_waiting_time.tv_nsec;
    long total_waiting_time_nsec =  waiting_time_nsec + waiting_time_sec * 1000000000;
    response.latency_time = total_waiting_time_nsec;
    return response;
};

/*initialize_thread_count() function:
  Adjusts the number of active threads using a mutex and signals a condition 
  if the value decreases.*/
void initialize_thread_count (int threads_counter) {
    pthread_mutex_lock(&threads_mutex);
    num_threads = num_threads + threads_counter;
    if (threads_counter < 0) {
        pthread_cond_signal(&full_cond_var);
    }
    pthread_mutex_unlock(&threads_mutex);
}

/*process_client_communication() function:
  Manage communication with a client by processing their request, sending a 
  response and updating the thread state.*/
void *process_client_communication(void *pointer) {
    int *thread_value = (int*)pointer;
    int id = thread_value[0];
    int client_socket = thread_value[1];
    struct request request = receive_client_request(client_socket);
    struct response response = process_client_request(request);
    if (send(client_socket, (void *)&response, sizeof(response), 0) < 0) {
        fprintf(stderr, "error while sending data to the socket\n");
        exit(EXIT_FAILURE);
    }
    close(threads[id]);
    pthread_mutex_lock(&fd_mutex);
    threads[id] = 0;
    pthread_mutex_unlock(&fd_mutex);
    initialize_thread_count(-1);
    free(thread_value);
    pthread_exit(NULL);
}

/*initialize_network_parameters() function:
  Set the process name, IP address and port of a connection.*/
void initialize_network_parameters(char name[6], char *ip, unsigned int port) {
    strcpy(process_name,name);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);
}

/*receive_server_response{} structure:
  Receive a response structure from the server socket and returns it.*/
struct response receive_server_response(int server_socket) {
    struct response received_response;
    if (recv(server_socket, (void *)&received_response, sizeof(received_response), 0) < 0) {
        fprintf(stderr, "error while receiving data from the socket\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }  
    return received_response;
};

/*process_server_communication() function:
  Establishes a connection with the server using a request, either reading or 
  writing, and ends up receiving said response.*/
void *process_server_communication(void *pointer) {
    char *response_mode;
    struct response response;
    struct request request;
    struct client_threads *client_threads = ((struct client_threads *)pointer);
    char *mode = client_threads->mode;
    int thread_id = client_threads->thread_id;
    enum operations action;
    action = WRITE;
    setbuf(stdout, NULL);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        fprintf(stderr, "error while creating socket\n");
        exit(EXIT_FAILURE); 
    }
    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0) {
        exit(EXIT_FAILURE);
    }
    if (strcmp(mode, "reader") == 0) {
        action = READ;
    }
    request.action = action;
    request.id = thread_id;
    if (send(server_socket, (void *)&request, sizeof(request), 0) < 0) {
        fprintf(stderr, "error while sending data to the socket\n");
        exit(EXIT_FAILURE);
    }
    response = receive_server_response(server_socket);
    if (response.action == READ) {
        response_mode = "Lector";
    } else {
        response_mode = "Escritor";   
    }
    fprintf(stdout, "[Cliente #%d] %s, contador=%d, tiempo=%ld ns\n", thread_id, response_mode, response.counter, response.latency_time);
    close(server_socket);
    pthread_exit(NULL);
}