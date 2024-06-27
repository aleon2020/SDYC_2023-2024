#include "stub.h"

int main(int argc, char **argv) {

    signal(SIGINT, shutdown_pubsub);
    int port, num_option = 0, index = 0;
    char *ip, *topic;

    static struct option long_options[] = {
        {"ip",      required_argument,  NULL,  'i' },
        {"port",    required_argument,  NULL,  'p' },
        {"topic",    required_argument,  NULL,  'm' },
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
                topic = optarg;
                break;
            default: 
                fprintf(stderr, "usage: ./subscriber --ip $BROKER_IP --port $BROKER_PORT --topic $TOPIC\n");
                exit(EXIT_FAILURE);
        }
    }

    initialize_network_parameters("Subscriber", ip, port);
    communicate_with_broker(topic, ip, port);
    exit(EXIT_SUCCESS);
}