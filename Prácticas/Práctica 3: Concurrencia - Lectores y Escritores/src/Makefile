all: server client

stub: stub.c 
	gcc -c stub.c -Wall -Wshadow -pthread

server: server.c stub.c
	gcc server.c stub.c -o server -Wall -Wshadow -pthread

client: client.c stub.c
	gcc client.c stub.c -o client  -Wall -Wshadow -pthread