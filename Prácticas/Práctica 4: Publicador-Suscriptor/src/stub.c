#include "stub.h"

struct sockaddr_in broker_address, pubsub_address;
struct topic topics_list[MAX_TOPICS];
pthread_mutex_t first_mutex, second_mutex, third_mutex;
pthread_cond_t just_cond_var;
int broker_socket, operation_mode, publisher_id, subscriber_id = 1;
int actual_subscribers, actual_topics;
int publisher_position, subscriber_position, just_num_threads;
struct sockaddr_in address;
char process_name[32], pubsub_topic[100];
int pubsub_socket, pubsub_id, client_type;

/*shutdown_broker() function:
  Handle specific signals by closing the server socket and ending the program.*/
void shutdown_broker(int signal) {
    close(broker_socket);
    exit(EXIT_FAILURE);
}

/*initialize_broker() function:
  Initializes a message server by configuring its socket, in addition
  of its IP address, its port and its mode of operation 
  (sequential, parallel, fair).*/
void initialize_broker(char *ip, int port, char *mode) {
    setbuf(stdout, NULL);
    broker_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (broker_socket < 0){
        fprintf(stderr, "error while creating socket\n");
        exit(EXIT_FAILURE); 
    }
    broker_address.sin_family = AF_INET;
    broker_address.sin_port = htons(port);
    broker_address.sin_addr.s_addr = inet_addr(ip);
    if (bind(broker_socket, (struct sockaddr*)&broker_address, sizeof(broker_address)) < 0) {
        fprintf(stderr, "error while binding socket\n");
        close(broker_socket);
        exit(EXIT_FAILURE); 
    }
    if (listen(broker_socket, 1000) < 0){
        fprintf(stderr, "error while listening socket\n");
        close(broker_socket);
        exit(EXIT_FAILURE);        
    }
    if (strcmp(mode, "secuencial") == 0) {
        operation_mode = SECUENCIAL;
    } else if (strcmp(mode, "paralelo") == 0) {
        operation_mode = PARALELO;
    } else if (strcmp(mode, "justo") == 0) {
        operation_mode = JUSTO;
    }
}

/*accept_pubsub_connection() function:
  Accepts incoming connections from the server, assigning the client socket
  to a thread and storing the information of said thread in an array.*/
void accept_pubsub_connection(int *thread_parameters) {
    socklen_t address_size = sizeof(pubsub_address);
    pubsub_socket = accept(broker_socket, (struct sockaddr*)&pubsub_address, &address_size);
    if (pubsub_socket < 0) {
        fprintf(stderr, "error while accepting socket\n");
        close(pubsub_socket);
        close(broker_socket);
        exit(EXIT_FAILURE);
    }
    thread_parameters[0] = pubsub_socket;
}

/*initialize_topic() function:
  Create a new topic if you still have space available, in addition to 
  check if the name of said topic already exists.*/
int initialize_topic(char *topic_name) {
    if (actual_topics < MAX_TOPICS) {
        for(int i = 0; i < actual_topics; i++) {
            if (strcmp(topics_list[i].name, topic_name) == 0) {
                return 2;
            }
        }
        struct topic topic;
        strcpy(topic.name, topic_name);
        topic.num_publishers = 0;
        topic.num_subscribers = 0;
        topics_list[actual_topics++] = topic;
        return 0;
    } else {
        return 1;
    }
}

/*update_pubsub_list() function:
  Updates the list of registered publishers and subscribers on a topic
  after unregistering one of them, sending a response message to the
  unregistered publisher/subscriber.*/
void update_pubsub_list(struct topic *topic, int action, int id) {
    int pubsub_position, i, fd;
    struct response response;
    if (action == UNREGISTER_SUBSCRIBER) {
        for (pubsub_position = 0; pubsub_position < MAX_SUBSCRIBERS; pubsub_position++) {
            if (topic->subscribers[pubsub_position].id == id) {
                fd = topic->subscribers[pubsub_position].fd;
                break;
            }  
        }
        for (i = pubsub_position; i < MAX_SUBSCRIBERS; i++) {
            topic->subscribers[i] = topic->subscribers[i+1];  
        }
    } else if (action == UNREGISTER_PUBLISHER) {
        for (pubsub_position = 0; pubsub_position < MAX_PUBLISHERS; pubsub_position++) {
            if (topic->publishers[pubsub_position].id == id) {
                fd = topic->publishers[pubsub_position].fd;
                break;
            }  
        }
        for (i = pubsub_position; i < MAX_PUBLISHERS; i++) {
            topic->publishers[i] = topic->publishers[i+1];  
        }
    }
    response.id = id;
    response.response_status = OK;
    if (send(fd, (void *)&response, sizeof(response), 0) < 0) {
        fprintf(stderr, "error while sending data\n");
        exit(EXIT_FAILURE);
    }
}

/*remove_pubsub() function:
  Removes a publisher or subscriber from a specific topic, as well as 
  update the list of publishers and subscribers, showing a summary
  of the operation carried out.*/
void remove_pubsub(int action, char *topic_name, int id) {
    struct timespec time;
    char pubsub_type[16];
    int topic_id = 0;
    for (topic_id = 0; topic_id < actual_topics; topic_id++) {
        if (strcmp(topics_list[topic_id].name, topic_name) == 0) {
            break;
        }
    }
    if (action == UNREGISTER_SUBSCRIBER) {
        strcpy(pubsub_type, "Suscriptor");
        update_pubsub_list(&topics_list[topic_id], action, id);
        topics_list[topic_id].num_subscribers--;
        subscriber_position--;
    } else if (action == UNREGISTER_PUBLISHER) {
        strcpy(pubsub_type, "Publicador");
        update_pubsub_list(&topics_list[topic_id], action, id);
        topics_list[topic_id].num_publishers--;
        publisher_position--;
    }
    clock_gettime(CLOCK_REALTIME, &time);
    long time_sec = time.tv_sec;
    long time_nsec = time.tv_nsec;
    fprintf(stdout, "[%ld.%ld] Eliminado Cliente (%d) %s : %s\n Resumen:\n", time_sec, time_nsec, id, pubsub_type, topic_name);
    for (int i = 0; i < actual_topics; i++) {
        fprintf(stdout, "\t%s: %d Suscriptores - %d Publicadores\n", topics_list[i].name, topics_list[i].num_subscribers, topics_list[i].num_publishers);
    }
    pthread_exit(NULL); 
}

/*sequential_message_distribution() function:
  Send a message to all subscribers of a topic in sequential order.*/
void sequential_message_distribution(struct message message) {
    struct publish publish;  
    int topics_socket, total_num_subscribers;
    strcpy(publish.data, message.data.data);
    publish.time_generated_data = message.data.time_generated_data;
    int topic_id = 0;
    for (topic_id = 0; topic_id < actual_topics; topic_id++) {
        if (strcmp(topics_list[topic_id].name, message.topic) == 0) {
            break;
        }
    }
    total_num_subscribers = topics_list[topic_id].num_subscribers;
    for (int i = 0; i < total_num_subscribers; i++) {
        topics_socket = topics_list[topic_id].subscribers[i].fd;
        if (send(topics_socket, (void *)&publish, sizeof(publish), 0) < 0) {
            fprintf(stdout, "error while sending data\n");
            exit(EXIT_FAILURE);
        }
    }
}

/*parallel_message_distribution() function:
  Send a message to a client concurrently using a thread.*/
void *parallel_message_distribution(void *pointer) {
    struct publish publish;
    struct send_message *message_pointer = (struct send_message*)pointer;
    struct send_message send_message = *message_pointer;
    strcpy(publish.data, send_message.message.data.data);
    publish.time_generated_data = send_message.message.data.time_generated_data;
    if (send(send_message.fd, (void *)&publish, sizeof(publish), 0) < 0) {
        fprintf(stderr, "error while sending data\n");
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
}

/*just_message_distribution() function:
  Send a message to all subscribers of a specific topic
  making sure all threads are ready before sending.*/
void *just_message_distribution(void *pointer) {
    struct send_message *message_pointer = (struct send_message*)pointer;
    struct send_message send_message = *message_pointer;
    struct publish publish;
    int topic_id = 0;
    for (topic_id = 0; topic_id < actual_topics; topic_id++) {
        if (strcmp(topics_list[topic_id].name, send_message.message.topic) == 0) {
            break;
        }
    }
    strcpy(publish.data, send_message.message.data.data);
    publish.time_generated_data = send_message.message.data.time_generated_data;
    pthread_mutex_lock(&first_mutex);
    just_num_threads++;
    if (just_num_threads < topics_list[topic_id].num_subscribers) {
        pthread_cond_wait(&just_cond_var, &first_mutex);
    }
    just_num_threads--;
    pthread_cond_broadcast(&just_cond_var);
    pthread_mutex_unlock(&first_mutex); 
    if (send(send_message.fd, (void *)&publish, sizeof(publish), 0) < 0) {
        fprintf(stderr, "error while sending data\n");
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
}

/*send_message_to_subscribers() function:
  Send a message to all subscribers of a specific topic
  depending on the operating mode used.*/
void send_message_to_subscribers(struct message message) {
    struct timespec time;
    int topic_id = 0;
    for (topic_id = 0; topic_id < actual_topics; topic_id++) {
        if (strcmp(topics_list[topic_id].name, message.topic) == 0) {
            break;
        }
    }
    int total_num_subscribers = topics_list[topic_id].num_subscribers;
    clock_gettime(CLOCK_REALTIME, &time);
    long time_sec = time.tv_sec;
    long time_nsec = time.tv_nsec;
    fprintf(stdout, "[%ld.%ld] Enviando mensaje en topic %s a %d suscriptores.\n", time_sec, time_nsec, message.topic, total_num_subscribers);
    if (operation_mode == SECUENCIAL) {
        sequential_message_distribution(message);
    } else if (operation_mode == PARALELO) {
        for (int i = 0; i < total_num_subscribers; i++) {
            struct send_message *send_message = malloc(256);
            pthread_t thread;
            send_message->message = message;
            send_message->fd = topics_list[topic_id].subscribers[i].fd;
            if (pthread_create(&thread, NULL, parallel_message_distribution, (void *) send_message) < 0) {
                fprintf(stderr, "error while creating thread\n");
                exit(EXIT_FAILURE);
            }
        } 
    } else if (operation_mode == JUSTO) {
        pthread_mutex_lock(&second_mutex);
        pthread_t threads[total_num_subscribers];
        for (int i = 0; i < total_num_subscribers; i++) {
            struct send_message *send_message = malloc(256);
            send_message->message = message;
            send_message->fd = topics_list[topic_id].subscribers[i].fd;
            if (pthread_create(&threads[i], NULL, just_message_distribution, (void *) send_message) < 0) {
                fprintf(stderr, "error while creating thread\n");
                exit(EXIT_FAILURE);
            }
        } 
        for (int i = 0; i < total_num_subscribers; i++) {
            pthread_join(threads[i], NULL);
        }
        pthread_mutex_unlock(&second_mutex);
    }
}

/*communicate_with_publisher() function:
  Manages communication with a publisher, that is, receives a set
  of messages to publish them in specific topics.*/
void communicate_with_publisher(int socket) {
    int action, id;
    char *received_data;
    struct message message;
    struct timespec initial_data_time, final_data_time;
    long initial_time_sec, final_time_sec, initial_time_nsec, final_time_nsec;
    while (1) {
        if (recv(socket, (void *)&message, sizeof(message), 0) < 0) {
            fprintf(stderr, "error while receiving data\n");
            exit(EXIT_FAILURE);
        }
        action = message.action;
        if (action == UNREGISTER_PUBLISHER) {
            id = message.id;
            remove_pubsub(action, message.topic, id);
        } else if (action == PUBLISH_DATA) {
            received_data = message.data.data;
            initial_data_time = message.data.time_generated_data;
            clock_gettime(CLOCK_REALTIME, &final_data_time);
            initial_time_sec = initial_data_time.tv_sec;
            initial_time_nsec = initial_data_time.tv_nsec;
            final_time_sec = final_data_time.tv_sec;
            final_time_nsec = final_data_time.tv_nsec;
            fprintf(stdout, "[%ld.%ld] Recibido mensaje para publicar en topic: %s - mensaje: %s - Generó %ld.%ld\n", final_time_sec, final_time_nsec, message.topic, received_data, initial_time_sec, initial_time_nsec);
            send_message_to_subscribers(message);
        } 
    }
}

/*communicate_with_subscriber() function:
  It manages communication with a subscriber, that is, it processes
  messages to unregister the subscriber of a specific topic.*/
void communicate_with_subscriber(int socket) {
    int action;
    struct message message;
    if (recv(socket, (void *)&message, sizeof(message), 0) < 0) {
        fprintf(stderr, "error while receiving data\n");
        exit(EXIT_FAILURE);
    }
    action = message.action;
    if (action == UNREGISTER_SUBSCRIBER) {
        char *topic = message.topic; 
        int id = message.id;
        remove_pubsub(action, topic, id);
    } 
}

/*communicate_with_pubsub() function:
  Manages communication with a client, that is, records 
  all the publishers and subscribers that reach you.*/
void *communicate_with_pubsub(void *pointer) {
    struct response response;
    struct message message;
    struct timespec time;
    int *thread_info = (int*)pointer;           
    int id, threads_socket = thread_info[0];
    char *pubsub_type, *topic;
    if (recv(threads_socket, (void *)&message, sizeof(message), 0) < 0) {
        fprintf(stderr, "error while receiving data from the socket\n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&third_mutex);
    clock_gettime(CLOCK_REALTIME, &time);
    long time_sec = time.tv_sec;
    long time_nsec = time.tv_nsec;
    if (message.action == REGISTER_SUBSCRIBER) {
        int topic_status = initialize_topic(message.topic);
        if (topic_status != 1){
            if (actual_subscribers < MAX_SUBSCRIBERS) {
                struct subscriber subscriber;
                subscriber.fd = threads_socket;
                strcpy(subscriber.topic, message.topic);
                subscriber.id = subscriber_id;
                int topic_id = 0;
                for (topic_id = 0; topic_id < actual_topics; topic_id++) {
                    if (strcmp(topics_list[topic_id].name, message.topic) == 0) {
                        break;
                    }
                }
                int subscribers_topic = topics_list[topic_id].num_subscribers;
                topics_list[topic_id].subscribers[subscribers_topic] = subscriber;
                id = subscriber_id;
                pubsub_type = "Suscriptor";
                topic = subscriber.topic;
                response.id = subscriber_id++;
                response.response_status = OK;
            } else {
                response.id = -1;
                response.response_status = LIMIT;  
            }
        } else if (topic_status == 1) {
            response.id = -1;
            response.response_status = ERROR; 
        }
    } else if (message.action == REGISTER_PUBLISHER) {
        int topic_id;
        int topic_exists = 0;
        for (topic_id = 0; topic_id < actual_topics; topic_id++) {
            if (strcmp(topics_list[topic_id].name, message.topic) == 0) {
                topic_exists = 1;
                break;
            }
        }
        if (topic_exists){
            if (publisher_id < MAX_PUBLISHERS) {
                struct publisher publisher;
                publisher.fd = threads_socket;
                strcpy(publisher.topic, message.topic);
                publisher.id = publisher_id;
                int publishers_topic = topics_list[topic_id].num_publishers;
                topics_list[topic_id].publishers[publishers_topic] = publisher;
                id = publisher_id;
                pubsub_type = "Publicador";
                topic = publisher.topic;
                response.id = publisher_id++;
                response.response_status = OK;
            } else {
                response.id = -1;
                response.response_status = LIMIT;    
            } 
        } else {
            response.id = -1;
            response.response_status = ERROR;      
        }
    }
    if (send(threads_socket, (void *)&response, sizeof(response), 0) < 0) {
        fprintf(stderr, "error while sending data to the socket\n");
        exit(EXIT_FAILURE);
    }
    if (response.id != -1) {
        fprintf(stdout, "[%ld.%ld] Nuevo Cliente (%d) %s conectado : %s\n Resumen:\n", time_sec, time_nsec, id, pubsub_type, topic);
        for (int i = 0; i < actual_topics; i++) {
            if (strcmp(topic, topics_list[i].name) == 0) {
                if (strcmp(pubsub_type, "Publicador") == 0) {
                    topics_list[i].num_publishers++;
                }  else {
                    topics_list[i].num_subscribers++;
                }
            }
            fprintf(stdout, "\t%s: %d Suscriptores - %d Publicadores\n", topics_list[i].name, topics_list[i].num_subscribers, topics_list[i].num_publishers);
        }
    }
    pthread_mutex_unlock(&third_mutex);
    if ((message.action == REGISTER_PUBLISHER) && (response.id != -1)) {
        communicate_with_publisher(threads_socket);
    } else if ((message.action == REGISTER_SUBSCRIBER) && (response.id != -1)) {
        communicate_with_subscriber(threads_socket);
    }
    pthread_exit(NULL);  
}

/*shutdown_pubsub() function:
  Handle specific signals by closing the client socket
  (all publishers and subscribers) and finishing the program.*/
void shutdown_pubsub(int signal) {
    struct timespec time;
    struct message message;
    struct response response;
    if (client_type == PUBLISHER) {
        message.action = UNREGISTER_PUBLISHER;
    } else {
        message.action = UNREGISTER_SUBSCRIBER;
    }
    strcpy(message.topic, pubsub_topic);
    message.id = pubsub_id;
    do {
        if (send(pubsub_socket, (void *)&message, sizeof(message), 0) < 0) {
            fprintf(stderr, "error while sending data to the socket\n");
            exit(EXIT_FAILURE);
        }
        if (recv(pubsub_socket, (void *)&response, sizeof(response), 0) < 0) {
            fprintf(stderr, "error while receiving data from the socket\n");
            exit(EXIT_FAILURE);
        }
    } while (response.response_status != OK);
    clock_gettime(CLOCK_REALTIME, &time);
    long time_sec = time.tv_sec;
    long time_nsec = time.tv_nsec;    
    fprintf(stdout, "[%ld.%ld] De-Registrado (%d) correctamente del broker.\n", time_sec, time_nsec, pubsub_id); 
    close(pubsub_socket);
    exit(EXIT_FAILURE);
}

/*initialize_network_parameters() function:
  Set the process name, IP address and port of a connection.*/
void initialize_network_parameters(char name[6], char *ip, unsigned int port) {
    strcpy(process_name,name);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip);
}

/*initialize_communication() function:
  Starts the data transfer between the server (broker) and the client
  (publisher and subscriber), which are published if they are
  from a publisher, or are received in the case of a subscriber, 
  in addition to calculating their latency in the latter case.*/
void initialize_communication() { 
    if (client_type == PUBLISHER) {
        struct publish publish;
        struct message message;
        struct timespec time;
        message.action = PUBLISH_DATA;
        FILE* ptr = fopen("/proc/loadavg", "r");
        while (1) {
            sleep(3);
            clock_gettime(CLOCK_REALTIME, &time);
            if (ptr != NULL) {
                fread(publish.data, sizeof(char), 100, ptr);
            }
            long time_sec = time.tv_sec;
            long time_nsec = time.tv_nsec;
            publish.time_generated_data = time;
            message.data = publish;
            strcpy(message.topic, pubsub_topic);
            fprintf(stdout, "[%ld.%ld] Publicado mensaje topic: %s - mensaje: %s - Generó %ld.%ld\n", time_sec, time_nsec, pubsub_topic, publish.data, time_sec, time_nsec);
            if (send(pubsub_socket, (void *)&message, sizeof(message), 0) < 0) {
                fprintf(stderr, "error while sending data to the socket\n");
                exit(EXIT_FAILURE);
            }
        }
    } else {
        struct publish publish;
        struct timespec time;
        long initial_latency_sec, initial_latency_nsec;
        float total_latency_time;
        while (1) {
            if (recv(pubsub_socket, (void *)&publish, sizeof(publish), 0) < 0) {
                fprintf(stderr, "error while receiving data from the socket\n");
                exit(EXIT_FAILURE);
            }
            clock_gettime(CLOCK_REALTIME, &time);
            long final_time_sec = time.tv_sec;
            long final_time_nsec = time.tv_nsec;
            long initial_time_sec = publish.time_generated_data.tv_sec;
            long initial_time_nsec = publish.time_generated_data.tv_nsec;
            if (final_time_nsec - initial_time_nsec < 0){
                initial_latency_sec = final_time_sec - initial_time_sec - 1;
                initial_latency_nsec = final_time_nsec - initial_time_nsec + 1000000000;
            } else {
                initial_latency_sec = final_time_sec - initial_time_sec;
                initial_latency_nsec = final_time_nsec - initial_time_nsec;
            }
            total_latency_time = initial_latency_sec + initial_latency_nsec / 1000000000.f;
            fprintf(stdout, "[%ld.%ld] Recibido mensaje topic: %s - mensaje: %s - Generó: %ld.%ld" "- Recibido: %ld.%ld - Latencia: %f.\n", time.tv_sec, time.tv_nsec, pubsub_topic, publish.data, initial_time_sec, initial_time_nsec, final_time_sec, final_time_nsec, total_latency_time); 
        }
    }
}

/*communicate_with_broker() function:
  Establish communication with the broker by registering everyone
  the publishers and subscribers that you have received, in order to start 
  then the data transfer process.*/
void communicate_with_broker(char *topic, char *ip, int port){
    if (strcmp(process_name, "Publisher") == 0) {
        client_type = PUBLISHER;
    } else {
        client_type = SUBSCRIBER;
    }
    strcpy(pubsub_topic, topic);
    struct timespec time;
    struct response response;
    struct message message;
    long time_sec, time_nsec;
    strcpy(message.topic, topic);
    if (client_type == PUBLISHER) {
        message.action = REGISTER_PUBLISHER;
    } else {
        message.action = REGISTER_SUBSCRIBER;    
    }
    setbuf(stdout, NULL);
    pubsub_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (pubsub_socket < 0) {
        fprintf(stderr, "error while creating socket\n");
        exit(EXIT_FAILURE); 
    }
    if (connect(pubsub_socket, (struct sockaddr *)&address, sizeof(address)) != 0) {
        fprintf(stderr, "error while connecting with the socket\n");
        exit(EXIT_FAILURE);
    }
    clock_gettime(CLOCK_REALTIME, &time);
    time_sec = time.tv_sec;
    time_nsec = time.tv_nsec;
    if (client_type == SUBSCRIBER) {
        fprintf(stdout, "[%ld.%ld] %s conectado con broker (%s:%d)\n", time_sec, time_nsec, process_name, ip, port);
    } else {
        fprintf(stdout, "[%ld.%ld] %s conectado con el broker correctamente.\n", time_sec, time_nsec, process_name);
    }
    if (send(pubsub_socket, (void *)&message, sizeof(message), 0) < 0){
        fprintf(stderr, "error while sending data to the socket\n");
        exit(EXIT_FAILURE);
    }
    if (recv(pubsub_socket, (void *)&response, sizeof(response), 0) < 0){
        fprintf(stderr, "error while receiving data from the socket\n");
        exit(EXIT_FAILURE);
    }
    pubsub_id = response.id;
    clock_gettime(CLOCK_REALTIME, &time);
    time_sec = time.tv_sec;
    time_nsec = time.tv_nsec;
    if (response.response_status == OK){
        fprintf(stdout, "[%ld.%ld] Registrado correctamente con ID: %d para topic %s\n",
        time_sec, time_nsec, response.id, topic);    
    } else {
        fprintf(stderr, "[%ld.%ld] Error al hacer el registro: error=%d\n", time_sec, time_nsec, response.response_status);
        exit(EXIT_FAILURE);       
    }
    initialize_communication();
}