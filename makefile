CFLAGS = -Wall
LDFLAGS = -lm
DEBUG_FLAGS = -g
# COMPILER = -std=gnu99

all: main

main: main.o ConexaoRawSocket.o
	c++ -o main main.o ConexaoRawSocket.o $(LDFLAGS) $(DEBUG_FLAGS)

main.o: main.cpp
	c++ -c main.cpp -o main.o $(CFLAGS)

ConexaoRawSocket.o: ConexaoRawSocket.cpp ConexaoRawSocket.h
	c++ -c ConexaoRawSocket.cpp $(CFLAGS)

clean:
	-rm -f *~ *.o
	
purge: clean
	-rm -f main