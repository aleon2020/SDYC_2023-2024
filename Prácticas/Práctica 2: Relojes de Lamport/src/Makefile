CC=gcc
DEPS = stub.h
CFLAGS= -lpthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: P1 P2 P3

P1: stub.o P1.o 
	$(CC) -o P1 stub.o P1.o $(CFLAGS)

P2: stub.o P2.o
	$(CC) -o P2 stub.o P2.o $(CFLAGS)

P3: stub.o P3.o
	$(CC) -o P3 stub.o P3.o $(CFLAGS)

clean:
	rm *.o
	rm P1 P2 P3
