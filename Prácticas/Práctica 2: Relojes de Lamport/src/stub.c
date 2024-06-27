/*Librerías*/
#include "stub.h"

/*Variables*/
#define CLIENTS 2
#define MAX_BUFFER_SIZE 1024
struct message msg;
struct sockaddr_in address;
char client_names[CLIENTS][MAX_BUFFER_SIZE];
pthread_t threads[CLIENTS];
int tcp_socket, conn_fd[CLIENTS], num_threads = 0, lamport_clock = 0;

/*Funciones: La explicación de cada función viene en el fichero stub.h*/
void signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("\nProcess shutting down...\n");
        exit(1);
    }
}

void set_parameters(char name[2], char* ip, unsigned int port) {
    strncpy(msg.origin, name, 2);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);
}

int init_server() {
    setbuf(stdout, NULL);
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        printf("Error creating socket\n");
        exit(1);
    }
    const int enable = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        exit(1);
    }
    int res = bind(tcp_socket, (struct sockaddr *) &address, sizeof(address));
    if (res < 0) {
        printf("Error binding socket\n");
        exit(1);
    }
    int list = listen(tcp_socket, CLIENTS);
    if (list < 0) {
        printf("Error listening to the socket\n");
        exit(1);
    }
}

void init_client() {
    setbuf(stdout, NULL);
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        printf("Error creating socket\n");
        exit(1);
    }
    if (connect(tcp_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("Error connecting with the socket\n");
        exit(1);
    }
}

int get_clock_lamport() {
    return lamport_clock;
}

void *init_communication() {
    if (recv(conn_fd[num_threads-1], &msg, sizeof(msg), 0) < 0) {
        printf("Error receiving data from the socket\n");
        pthread_exit(NULL);
    }
    if (get_clock_lamport() < msg.clock_lamport) {
        lamport_clock = msg.clock_lamport;
    }
    lamport_clock++;
    msg.clock_lamport = get_clock_lamport();
    strncpy(client_names[num_threads-1], msg.origin, sizeof(msg.origin));
    printf("%s, %d, RECV (%s), %s\n", msg.origin, get_clock_lamport(), msg.origin, "READY_TO_SHUTDOWN");
    pthread_exit(NULL);
}

void accept_communication() {
    struct sockaddr sock_cli;
    socklen_t len = sizeof(sock_cli);
    conn_fd[num_threads] = accept(tcp_socket, (struct sockaddr *) &sock_cli, &len);
    if (conn_fd[num_threads] < 0) {
        printf("Error accepting the socket\n");
        exit(1);
    }
    if (pthread_create(&threads[num_threads], NULL, init_communication, NULL) != 0) {
        printf("Error creating thread\n");
        exit(1);
    }
    num_threads++;
}

void ready_to_shutdown_message() {
    lamport_clock++;
    msg.clock_lamport = get_clock_lamport();
    if (send(tcp_socket, &msg, sizeof(msg), 0) < 0) {
        printf("Error sending data to the socket\n");
        exit(1);
    }  
    printf("%s, %d, SEND, %s\n", msg.origin, msg.clock_lamport, "READY_TO_SHUTDOWN");
}

void *client_receive() {
    if (recv(tcp_socket, &msg, sizeof(msg), 0) < 0) {
        printf("Error receiving data from the socket\n");
        pthread_exit(NULL);
    }
    if (get_clock_lamport() < msg.clock_lamport) {
        lamport_clock = msg.clock_lamport;
    }
    lamport_clock++;
    msg.clock_lamport = get_clock_lamport();
    strncpy(client_names[0], msg.origin, sizeof(msg.origin));
    printf("%s, %d, RECV (%s), %s\n", msg.origin, get_clock_lamport(), msg.origin, "SHUTDOWN_NOW");
    pthread_exit(NULL);
}

void waiting_message_from_server() {
    if (pthread_create(&threads[0], NULL, client_receive, NULL) != 0) {
        printf("Error creating thread\n");
        exit(1);
    }
    if (pthread_join(threads[0], NULL) != 0) {
        printf("Error joining thread\n");
        exit(1);
    }
}

void *server_receive(void *index) {
    int *process_index = index;
    if (recv(conn_fd[*process_index], &msg, sizeof(msg), 0) < 0) {
        printf("Error receiving data from the socket\n");
        pthread_exit(NULL);
    }
    if (get_clock_lamport() < msg.clock_lamport) {
        lamport_clock = msg.clock_lamport;
    }
    lamport_clock++;
    msg.clock_lamport = get_clock_lamport();
    strncpy(client_names[*process_index], msg.origin, sizeof(msg.origin));
    printf("%s, %d, RECV (%s), %s\n", msg.origin, get_clock_lamport(), msg.origin, "SHUTDOWN_ACK");
    pthread_exit(NULL);
}

void find_process(char *name, int *index) {
    for(int i = 0; i < num_threads; i++) {
        if (client_names[i][1] == name[1]) {
            *index = i;
        }
    }
}

void waiting_message_from_client() {
    int index;
    if (get_clock_lamport() == 4) {
        find_process("P1", &index);
    } 
    if (get_clock_lamport() == 8) {
        find_process("P3", &index);
    } 
    if (pthread_create(&threads[0], NULL, server_receive, &index) != 0) {
        printf("Error creating thread\n");
        exit(1);
    }
    if (pthread_join(threads[0], NULL) != 0) {
        printf("Error joining thread\n");
        exit(1);
    }
}

void shutdown_now_message() {
    int index;
    if (get_clock_lamport() < 3) {
        for(int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }    
    }
    if (get_clock_lamport() == 3) {
        find_process("P1", &index);
    } 
    if (get_clock_lamport() == 7) {
        find_process("P3", &index);
    } 
    lamport_clock++;
    msg.clock_lamport = get_clock_lamport();
    if (send(conn_fd[index], &msg, sizeof(msg), 0) < 0) {
        printf("Error sending data to the socket\n");
        exit(1);
    }
    printf("%s, %d, SEND, %s\n", msg.origin, msg.clock_lamport, "SHUTDOWN_NOW");
}

void shutdown_ack_message() {
    lamport_clock++;
    msg.clock_lamport = get_clock_lamport();
    if (send(tcp_socket, &msg, sizeof(msg), 0) < 0) {
        printf("Error sending data to the socket\n");
        exit(1);
    }
    printf("%s, %d, SEND, %s\n", msg.origin, msg.clock_lamport, "SHUTDOWN_ACK");
}

void close_server() {
    for (int i = 0; i < num_threads; i++) {
        close(conn_fd[i]);
    }
    close(tcp_socket);
}

void close_client() {
    close(tcp_socket);
}
