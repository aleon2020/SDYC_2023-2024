/*Librerías*/
#include "stub.h"

/*Función principal main*/
int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    if (argc != 3){
        printf("usage: ./P1 [IP] [PORT]\n");
        exit(1);
    };
    char* ip = argv[1];
    unsigned int port = atoi(argv[2]);
    set_parameters("P1", ip, port);
    init_client();
    ready_to_shutdown_message();
    waiting_message_from_server();
    shutdown_ack_message();
    close_client();
    exit(0);
}
