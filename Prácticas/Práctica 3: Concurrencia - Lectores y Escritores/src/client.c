#include "stub.h"

int main(int argc, char **argv) {

    int port, num_threads, num_option = 0, index = 0;
    char *ip, *mode;

    static struct option long_options[] = {
        {"ip",      required_argument,  NULL,  'i' },
        {"port",    required_argument,  NULL,  'p' },
        {"mode",    required_argument,  NULL,  'm' },
        {"threads", required_argument,  NULL,  't' },
        {0,         0,                  0,     0   }
    };

    while ((num_option = getopt_long_only(argc, argv, "", long_options, &index)) != -1) {
        switch (num_option) {
            case 'i':
                ip = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                mode = optarg;
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            default: 
                fprintf(stderr, "usage: ./client --ip IP --port PORT --mode writer/reader --threads N\n");
                exit(EXIT_FAILURE);
        }
    }

    if (num_threads <= 0 || (strcmp(mode, "writer") != 0 && strcmp(mode, "reader") != 0)) {
        fprintf(stderr, "usage: ./client --ip IP --port PORT --mode writer/reader --threads N\n");
        exit(EXIT_FAILURE);
    }
    
    struct client_threads client_threads[num_threads];
    pthread_t threads[num_threads];
    initialize_network_parameters("Client", ip, port);

    for (int i = 0; i < num_threads; i++) {
        client_threads[i].mode = mode;
        client_threads[i].thread_id = i;
        if (pthread_create(&threads[i], NULL, process_server_communication, (void *)&client_threads[i]) < 0){
            fprintf(stderr, "error while creating thread\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) < 0) {
            fprintf(stderr, "error while creating thread\n");
            exit(EXIT_FAILURE);
        };
    }
    exit(EXIT_SUCCESS);
}