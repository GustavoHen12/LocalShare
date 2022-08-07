#include<stdio.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include"socket_utils.h"


int main(int argc, char const* argv[]){
    char *mode = "lo";
    int client_socket = conect_raw_socket(mode);

    char* hello = "Hello from client";
    char buffer[1024] = { 0 };

    send(client_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    
    read(client_socket, buffer, 1024);
    close(client_socket);
    
    printf("%s\n", buffer);
    return 0;
}
