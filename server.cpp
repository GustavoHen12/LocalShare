#include<stdlib.h>
#include <bits/stdc++.h>

#include"socket_utils.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#define PORT 8080

using namespace std;

int main(int argc, char const* argv[]){
    char *mode = "lo";
    int server_socket = conect_raw_socket(mode);
    // int server_fd = ConexaoRawSocket("lo");

    char buffer[1024] = { 0 };
    
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    vector<char> buf(5000);
    int bytes = recv(server_socket, buf.data(), buf.size(), 0);
    cout << bytes << "\n";
    for(int i = 0; i < bytes; i++)
        cout << buf[i];
    
    
    // recv(new_socket, buffer, 1024);
    // printf("%s\n", buffer);

    // close(new_socket);
    // closing the listening socket
    shutdown(server_socket, SHUT_RDWR);
    return 0;
}
