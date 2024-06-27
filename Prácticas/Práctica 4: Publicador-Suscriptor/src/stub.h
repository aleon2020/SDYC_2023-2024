#ifndef STUB_H
#define STUB_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <getopt.h>
#include <semaphore.h>

#define PUBLISHER 0
#define SUBSCRIBER 1

// MAXIMUM BROKER PARAMETERS
#define MAX_TOPICS 10
#define MAX_PUBLISHERS 100
#define MAX_SUBSCRIBERS 900

// BROKER OPERATING MODES

// Sequential: Every time it receives a message from a publisher, it will send
// sequentially that message to registered subscribers to that topic 
// following a FIFO policy. Being sequential, until the end of the 
// communication with one subscriber cannot pass to the next. It is the way 
// default broker operation.
#define SECUENCIAL 0

// Parallel: Every time it receives a message from a publisher, it will send 
// parallel the message to registered subscribers to that topic.
#define PARALELO 1

// Fair: Whenever you receive a message from a publisher, you should make sure 
// that all subscribers receive it “at the same time”. Use the 
// synchronization mechanisms seen in class that best suit your 
// needs.
#define JUSTO 2

/*operations{} enumeration:
  REGISTER_PUBLISHER: Register a publisher.
  UNREGISTER_PUBLISHER: Unregisters a publisher.
  REGISTER_SUBSCRIBER: Register a subscriber.
  UNREGISTER_SUBSCRIBER: Unregisters a subscriber.
  PUBLISH_DATA: Publish a data set.*/
enum operations {
    REGISTER_PUBLISHER = 0,
    UNREGISTER_PUBLISHER,
    REGISTER_SUBSCRIBER,
    UNREGISTER_SUBSCRIBER,
    PUBLISH_DATA
};

/*publish{} structure:
  time_generated_data: Indicates the timestamp at which the data was generated.
  data: Contains all the data that has been published.*/
struct publish {
    struct timespec time_generated_data;
    char data[100];
};

/*message{} structure:
  action: Indicates the action to be performed (enumerated options).
  topic: Indicates the name of the topic associated with the message.
  id: Indicates the identifier used in deregistration messages.
  data: Indicates what data has been published.*/
struct message {
    enum operations action;
    char topic[100];
    // Only used in UNREGISTER messages
    int id;
    // Only used in PUBLISH_DATA messages
    struct publish data;
};

/*status{} enumeration:
  ERROR: Indicates that an error has occurred,
  LIMIT: Indicates whether the maximum number of
  publishers/subscribers/topics that can be stored.
  OK: Indicates that the registration was successful.*/
enum status {
    ERROR = 0,
    LIMIT,
    OK
};

/*response{} structure:
  response_status: Indicates the status of the response, defined in the 
  status enumeration.
  id: Indicates the identifier that is associated with the response.*/
struct response {
    enum status response_status;
    int id;
};

/*publisher{} structure:
  topic: Indicates the topic to which the publisher subscribes.
  id: Indicates the publisher's identifier.
  id_in_topic: Indicates the identifier of the publisher within the topic.
  fd: Indicates which file descriptor is associated with the publisher.*/
struct publisher {
    char topic[100];
    int id;
    int id_in_topic;
    int fd;
};

/*subscriber{} structure:
  topic: Indicates the topic to which you are subscribed.
  id: Indicates the subscriber identifier.
  id_in_topic: Indicates the subscriber identifier within the topic.
  fd: Indicates which file descriptor is associated with the subscriber.*/
struct subscriber {
    char topic[100];
    int id;
    int id_in_topic;
    int fd;
};

/*topic{} structure:
  name: Indicates the name of the topic.
  data: Indicates what data is associated with the topic.
  num_subscribers: Indicates the number of subscribers that the topic has.
  num_publishers: Indicates the number of publishers that the topic has.
  Subscribers: Indicates which subscribers the topic has.
  publishers: Indicates which publishers own the topic.*/
struct topic {
    char name[100];
    char data[1024];
    int num_subscribers;
    int num_publishers;
    struct subscriber subscribers[MAX_SUBSCRIBERS];
    struct publisher publishers[MAX_PUBLISHERS];
};

/*send_message{} structure:
  message: Indicates the message to be sent.
  fd: Indicates the file descriptor associated with said communication.*/
struct send_message {
    struct message message;
    int fd;
};

// BROKER (SERVER) FUNCTIONS AND STRUCTURES
void shutdown_broker(int signal);
void initialize_broker(char *ip, int port, char *mode);
void accept_pubsub_connection(int *thread_parameters);
int initialize_topic(char *topic_name);
void update_pubsub_list(struct topic *topic, int action, int id);
void remove_pubsub(int action, char *topic_name, int id);
void sequential_message_distribution(struct message message);
void *parallel_message_distribution(void *pointer);
void *just_message_distribution(void *ptr);
void send_message_to_subscribers(struct message message);
void communicate_with_publisher(int socket);
void communicate_with_subscriber(int socket);
void *communicate_with_pubsub(void *pointer);

// PUBLISHER/SUBSCRIBER (CLIENT) FUNCTIONS AND STRUCTURES
void shutdown_pubsub(int signal);
void initialize_network_parameters(char name[6], char *ip, unsigned int port);
void initialize_communication();
void communicate_with_broker(char *topic, char *ip, int port);

#endif