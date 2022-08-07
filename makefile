# Source files
CPP_SOURCE=$(wildcard *.cpp)
H_SOURCE=$(wildcard *.h)

CPP_SOURCE_CLIENT=commands.cpp protocol.cpp socket_utils.cpp client.cpp
H_SOURCE_CLIENT=commands.h protocol.h socket_utils.h

CPP_SOURCE_SERVER=commands.cpp protocol.cpp socket_utils.cpp server.cpp
H_SOURCE_SERVER=commands.h protocol.h socket_utils.h

OBJ_CLIENT=$(CPP_SOURCE_CLIENT:.cpp=.o)
OBJ_SERVER=$(CPP_SOURCE_SERVER:.cpp=.o)

# Compiler
CC=c++

# Flags
CC_FLAGS =-Wall -lm
DEBUG_FLAGS = -g


# Compilacao
all: client server

client: $(OBJ_CLIENT)
	$(CC) -o $@ $^

server: $(OBJ_SERVER)
	$(CC) -o $@ $^

%.o: %.c %.h
	$(CC) -o $@ $< $(CC_FLAGS)

server.o: server.cpp $(H_SOURCE_SERVER)
	$(CC) -c $@ $^

client.o: client.cpp  $(H_SOURCE_CLIENT)
	$(CC) -c $@ $^

clean:
	-rm -f *~ *.o *.h.gch
	
purge: clean
	-rm -f main