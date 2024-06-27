/*Librerías*/
#ifndef STUB_H
#define STUB_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <err.h>

/*- READY_TO_SHUTDOWN: La máquina notifica que está lista para apagarse.
  - SHUTDOWN_NOW: La máquina sabe que debe apagarse.
  - SHUTDOWN_ACK: La máquina manda este mensaje justo antes del shutdown.*/
enum operations {
    READY_TO_SHUTDOWN = 0,  
    SHUTDOWN_NOW,  
    SHUTDOWN_ACK    
};

/*Estructura utilizada para los mensajes intercambiados mediante sockets:
  - Campo origin: Contiene el nombre del proceso que envía el mensaje.
  - Campo action: Contiene los valores del enumerado options.
  - Campo clock_lamport: Envía el contador de Lamport.*/
struct message {
    char origin[20];
    enum operations action;
    unsigned int clock_lamport; 
};

/*Cierra un proceso abierto pulsando Ctrl+C.*/
void signal_handler(int signal);

/*Establece el nombre del proceso, la dirección IP y el puerto.*/
void set_parameters(char name[2], char* ip, unsigned int port);

/*Inicia la conexión del socket servidor (P2).*/
int init_server();

/*Inicia la conexión de los sockets cliente (P1 y P3).*/
void init_client();

/*Obtiene el valor del reloj de Lamport.*/
int get_clock_lamport();

/*Verifica que P1 y P3 han notificado a P2 que están listos para apagarse.*/
void *init_communication();

/*Extrae la primera petición de la cola deconexiones pendientes,
  y crea un nuevo socket para cada cliente. */
void accept_communication();

/*READY_TO_SHUTDOWN: La máquina notifica que está lista para apagarse.*/
void ready_to_shutdown_message();

/*Verifica que el cliente ha recibido un mensaje del servidor.*/
void *client_receive();

/*El cliente espera un mensaje del servidor.*/
void waiting_message_from_server();

/*Verifica que el servidor ha recibido un mensaje del cliente.*/
void *server_receive(void *index);

/*Indica a qué proceso se le envía / llega un mensaje
  según el valor del reloj de Lamport.*/
void find_process(char *name, int *index);

/*El servidor espera un mensaje del cliente.*/
void waiting_message_from_client();

/*SHUTDOWN_NOW: La máquina sabe que debe apagarse.*/
void shutdown_now_message();

/*SHUTDOWN_ACK: La máquina manda este mensaje justo antes del shutdown.*/
void shutdown_ack_message();

/*Cierra el servidor.*/
void close_server();

/*Cierra el cliente.*/
void close_client();

#endif
