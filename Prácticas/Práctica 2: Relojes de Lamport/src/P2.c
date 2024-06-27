/*Librerías*/
#include "stub.h"

/*Función principal main*/
int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    if (argc != 3){
        printf("usage: ./P2 [IP] [PORT]\n");
        exit(1);
    };
    char* ip = argv[1];
    unsigned int port = atoi(argv[2]);
    set_parameters("P2", ip, port);
    init_server();
    accept_communication();
    accept_communication();
    shutdown_now_message();
    waiting_message_from_client();
    shutdown_now_message();
    waiting_message_from_client();
    close_server();
    /*El proceso P2 escribe este mensaje cuando sabe que los otros 2 procesos 
      han realizado el shutdown*/
    printf("Los clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());
    exit(0);
}
