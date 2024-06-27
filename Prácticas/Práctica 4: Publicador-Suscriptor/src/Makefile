all: broker subscriber publisher

stub: stub.c 
	gcc -c stub.c -Wall -Wshadow -pthread

broker: broker.c stub.c
	gcc broker.c stub.c -o broker -Wall -Wshadow -pthread
	
subscriber: subscriber.c stub.c
	gcc subscriber.c stub.c -o subscriber -Wall -Wshadow -pthread

publisher: publisher.c stub.c
	gcc publisher.c stub.c -o publisher -Wall -Wshadow -pthread