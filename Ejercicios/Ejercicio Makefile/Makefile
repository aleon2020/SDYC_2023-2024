CC=gcc
CFLAGS=-I.
DEPS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: helloworld.o library.o helloworld2
	$(CC) -o helloworld helloworld.o library.o 


helloworld2: helloworld2.c 
	gcc -g -Wall -o helloworld2 helloworld2.c

clean:
	rm -f *.o helloworld helloworld2