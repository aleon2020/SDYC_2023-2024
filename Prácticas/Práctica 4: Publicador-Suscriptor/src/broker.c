#include "stub.h"

int main(int argc, char **argv) {

    signal(SIGINT, shutdown_broker);
    int port, num_option = 0, index = 0;
    char *operation_mode, *ip = "0.0.0.0";

    static struct option long_options[] = {
        {"port",    required_argument,  NULL,  'p' },
        {"mode",    required_argument,  NULL,  'm' },
        {0,         0,                  0,     0   }
    };

    while ((num_option = getopt_long_only(argc, argv, "", long_options, &index )) != -1) {
        switch (num_option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'm': 
                operation_mode = optarg;
                break;
            default: 
                fprintf(stderr, "usage: ./broker --port $BROKER_PORT --mode $MODE\n");
                exit(EXIT_FAILURE);
        }
    }

    if ((strcmp(operation_mode, "secuencial") != 0 && strcmp(operation_mode, "paralelo") != 0 && strcmp(operation_mode, "justo") != 0)) {
        fprintf(stderr, "usage: ./broker --port $BROKER_PORT --mode $MODE\n");
        exit(EXIT_FAILURE);
    }
    
    initialize_broker(ip, port, operation_mode);

    while (1) {
        pthread_t new_thread;
        int *thread_info = malloc(16);
        accept_pubsub_connection(thread_info);
        if (pthread_create(&new_thread, NULL, communicate_with_pubsub, (void *)thread_info) < 0) {
            fprintf(stderr, "error while creating thread\n");
            exit(EXIT_FAILURE);
        }
    }
}