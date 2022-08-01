#include<stdio.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#include"socket_utils.h"


int main(int argc, char const* argv[]){
    int client_socket = ConexaoRawSocket("lo");

    char* hello = "Hello from client";
    char buffer[1024] = { 0 };

    send(client_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    
    read(client_socket, buffer, 1024);
    close(client_socket);
    
    printf("%s\n", buffer);
    return 0;
}
