/*Librerías*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>

/*Variables*/
#define PORT 8081
#define MAX_BUFFER_SIZE 1024

/*Función que permite finalizar el proceso pulsando Ctrl+C*/
void end_process(int sign) {
    if (sign == SIGINT) {
        printf("\nClient shutting down...\n");
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

	/*Establecemos conexión con el servidor (si devuelve un entero negativo da error)*/
	int c = connect(tcp_socket ,(struct sockaddr *) &server_addr, sizeof(server_addr));
	if (c == -1) {
		printf("\nError connecting with the socket\n");
        exit(1);
	}
	printf("connected to the server...\n");

	char buff[MAX_BUFFER_SIZE];

	while (1) {
		/*Enviamos datos por el socket*/
		printf("> ");
		fgets(buff, MAX_BUFFER_SIZE, stdin);
		send(tcp_socket, buff, sizeof(buff), 0);
		/*Recibimos datos del socket*/
		recv(tcp_socket, (void*) buff, sizeof(buff), 0);
		printf("+++ %s", buff);
	}
	exit(0);
}
