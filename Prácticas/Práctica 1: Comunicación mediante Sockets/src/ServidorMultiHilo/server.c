/*Librerías*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/*Variables*/
#define PORT 8081
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 100

struct thread_params {
	int tcp_socket;
	char *string;
};

/*Función que envía "Hello client!" por cada mensaje del cliente*/
void *thread_client(void *args) {
	char buff[MAX_BUFFER_SIZE];
	struct thread_params *params = args;
	int conn_fd =  params->tcp_socket;
	recv(conn_fd, buff, sizeof(buff), MSG_DONTWAIT);
	printf("> %s\n", buff);
	char *assent = "Hello client!";
	/*El servidor espera un tiempo aleatorio en el rango 0.5-2 segundos*/
	usleep((rand() % (1500000)) + 500000);
	send(conn_fd, assent, MAX_BUFFER_SIZE, 0);
	pthread_exit(NULL);
}

/*Función que permite finalizar el proceso pulsando Ctrl+C*/
void end_process(int sign) {
    if (sign == SIGINT) {
        printf("\nServer shutting down...\n");
        exit(0);
    }
}

/*Función principal main*/
int main(int argc, char *argv[]) {

	/*Deshabilitamos el buffering a la hora de imprimir mensajes*/
	setbuf(stdout, NULL);

	signal(SIGINT, end_process);

	/*El puerto se le pasa como primer argumento al servidor (./server [PORT])*/
	short port = atoi(argv[1]);

	/*Definimos los detalles de la conexión (tipo de socket, puerto, IP)*/
	struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = port;

	/*Creamos el socket (si devuelve un entero negativo da error)*/
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket == -1) {
		printf("\nError creating socket\n");
        exit(1);
	}
	printf("Socket successfully created...\n");

	/*Forzamos al SO a la reutilización instantánea del socket*/
	const int enable = 1;
	if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		printf("\nError forcing the operative system\n");
        exit(1);
	}

	/*Asignamos una dirección al socket (si devuelve un entero negativo da error)*/
	int res = bind(tcp_socket, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	if (res == -1) {
		printf("\nError binding socket\n");
        exit(1);
	}
	printf("Socket successfully binded...\n");

	/*Convertimos el socket definido por el descriptor en un socket pasivo*/
	int list = listen(tcp_socket, MAX_BUFFER_SIZE);
	if (list == -1) {
		printf("\nError listening to the socket\n");
        exit(1);
	}
	printf("Server listening...\n");

	/*Variables creadas para el uso de hilos*/
	char buff[MAX_BUFFER_SIZE];
	pthread_t threads[MAX_CLIENTS];
	int *sockets[MAX_CLIENTS];
	struct thread_params params;
	int current = 0;

	while (1) {
		while (current < MAX_CLIENTS) {

			/*Extraemos la primera petición de la cola de conexiones pendientes,
	  		  y se crea un nuevo socket para el cliente*/
			struct sockaddr_in sock_cli;
			socklen_t len = sizeof(sock_cli);
			int conn_fd = accept(tcp_socket, (struct sockaddr *) &sock_cli, &len);
			if (conn_fd == -1) {
				printf("\nError accepting the socket\n");
        		exit(1);
			}

			/*Creamos un bloque de memoria*/
			sockets[current] = malloc(sizeof(conn_fd));
			*sockets[current] = conn_fd;
			params.tcp_socket = *sockets[current];
			params.string = buff;

			/*Vamos creando los threads para irlos poniendo en ejecución*/
			if (pthread_create(&threads[current], NULL, &thread_client, (void *) sockets[current]) < 0) {
				printf("\nError creating thread\n");
				exit(1);
			}
			current++;
		}
		
		/*Realizamos una espera hasta que un thread determinado haya finalizado*/
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (pthread_join(threads[i], NULL) < 0) {
				printf("\nError joining thread\n");
        		exit(1);
			}
			/*Marcamos el socket como cerrado*/
			close(*sockets[i]);
			/*Eliminamos el bloque de memoria creado previamente*/
			free(sockets[i]);
		}
		current = 0;
	}
	exit(0);
}
