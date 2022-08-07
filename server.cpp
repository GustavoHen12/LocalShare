#include<stdlib.h>
#include <bits/stdc++.h>

// #include"socket_utils.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include"protocol.h"

#define PORT 8080

using namespace std;

int main(int argc, char const* argv[]){
    char *mode = "lo";
    int server_socket = conect_raw_socket(mode);
    init_protocol(SERVER, server_socket, 2, 0);
    
    ifstream ifNull;
    vector<u_int8_t> vecNull;

    while(1){
        msg_t *msg = get_message();
        // if(msg->type != DATA_TYPE){
        //     send_message(ACK_TYPE, ifNull, vecNull);
        // }

        for(int i = 0; i < msg->size; i++){
            char c = (msg->data_bytes)[i];
            cout << c;
        }
        cout << endl;
    }
    
    // recv(new_socket, buffer, 1024);
    // printf("%s\n", buffer);

    // close(new_socket);
    // closing the listening socket
    shutdown(server_socket, SHUT_RDWR);
    return 0;
}
