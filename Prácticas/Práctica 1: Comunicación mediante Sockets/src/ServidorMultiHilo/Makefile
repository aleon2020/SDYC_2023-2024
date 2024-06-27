CC=gcc
CFLAGS=-I -lpthread.
DEPS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
all: server client

server: server.o
	$(CC) -o server server.o $(CFLAGS)
	
client: client.o
	$(CC) -o client client.o $(CFLAGS)

clean:
	rm -f *.o
