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

	/*Le pasamos como argumentos un ID, la IP y el puerto
	  (./client [ID] [IP] [PORT])*/
	char* id = argv[1];
	long ip = atoi(argv[2]);
	short port = atoi(argv[3]);

	/*Definimos los detalles de la conexión (tipo de socket, puerto, IP)*/
	struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ip;
    server_addr.sin_port = port;

	/*Creamos el socket (si devuelve un entero negativo da error)*/
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket == -1) {
		printf("\nError creating socket\n");
		exit(1);
	}
	printf("Socket successfully created...\n");

	/*Muestra un string por cada mensaje enviado al servidor*/
	char buff[MAX_BUFFER_SIZE] = "Hello server! From client: ";

	/*Creamos un bloque de memoria*/
	char *send_msg = malloc(strlen(buff) + strlen(argv[1]) + 1);
	strcat(buff, id);
	strcpy(send_msg, buff);

	/*Establecemos conexión con el servidor (si devuelve un entero negativo da error)*/
	int c = connect(tcp_socket ,(struct sockaddr *) &server_addr, sizeof(server_addr));
	if (c == -1) {
		printf("\nError connecting with the socket\n");
        exit(1);
	}
	printf("connected to the server...\n");

	/*Enviamos datos por el socket*/
	send(tcp_socket, send_msg, sizeof(buff), 0);
	char buff2[MAX_BUFFER_SIZE];
	/*Recibimos datos del socket*/
	recv(tcp_socket, buff2, sizeof(buff2), 0);
	/*Eliminamos el bloque de memoria creado previamente*/
	free(send_msg);
	exit(0);
}
