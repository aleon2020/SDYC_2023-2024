#include "stub.h"

int main(int argc, char **argv) {

    int counter, port, num_option = 0, index = 0;
    char *priority, *ip = "0.0.0.0";
    char buffer[20];

    static struct option long_options[] = {
        {"port",    required_argument,  NULL,  'p' },
        {"priority",    required_argument,  NULL,  'm' },
        {0,         0,                  0,     0   }
    };

    while ((num_option = getopt_long_only(argc, argv, "", long_options, &index)) != -1) {
        switch (num_option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                priority = optarg;
                break;
            default: 
                fprintf(stderr, "usage: ./server --port PORT --priority writer/reader\n");
                exit(EXIT_FAILURE);
        }
    }

    if ((strcmp(priority, "writer") != 0 && strcmp(priority, "reader") != 0)) {
        fprintf(stderr, "usage: ./server --port PORT --priority writer/reader\n");
        exit(EXIT_FAILURE);
    }
    
    initialize_server(ip, port, priority);
    FILE *file = fopen("server_output.txt",  "r");
    if (file != NULL) {
        while (fgets(buffer, sizeof(buffer), file) != NULL) {}
        fclose(file);
        counter = atoi(buffer);
    } else {
        file = fopen("server_output.txt",  "a");
        fclose(file);
        counter = 0;
    }
    initialize_counter(counter);

    while (1) {
        pthread_t new_thread;
        int *thread_info = malloc(50);
        initialize_client_thread(thread_info);
        if (pthread_create(&new_thread, NULL, process_client_communication, (void *)thread_info) < 0) {
            fprintf(stderr, "error while creating thread\n");
            exit(EXIT_FAILURE);
        }
        initialize_thread_count(1);
    }
}