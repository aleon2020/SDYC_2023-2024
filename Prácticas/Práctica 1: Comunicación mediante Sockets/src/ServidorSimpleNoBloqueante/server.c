/*Librerías*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

/*Variables*/
#define PORT 8081
#define MAX_BUFFER_SIZE 1024

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

	/*Definimos los detalles de la conexión (tipo de socket, puerto, IP)*/
	struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

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

	/*Asignamos una dirección al socket (si devuelve un entero no negativo da error)*/
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

	/*Extraemos la primera petición de la cola de conexiones pendientes,
	  y se crea un nuevo socket para el cliente*/
	struct sockaddr_in sock_cli;
	socklen_t len = sizeof(sock_cli);
	int conn_fd = accept(tcp_socket ,(struct sockaddr *) &sock_cli, &len);
	if (conn_fd == -1) {
		printf("ERROR: when calling accept...\n");
        exit(1);
	}

	char buff[MAX_BUFFER_SIZE];

	while (1) {
		/*Recibimos datos del socket*/
		recv(conn_fd, (void*) buff, sizeof(buff), 0);
		printf("+++ %s", buff);
		printf("> ");
		/*Enviamos datos por el socket*/
		fgets(buff, MAX_BUFFER_SIZE, stdin);
		send(conn_fd, buff, sizeof(buff), 0);
	}
	/*Marcamos el socket como cerrado*/
	close(conn_fd);
	exit(EXIT_SUCCESS);
}
