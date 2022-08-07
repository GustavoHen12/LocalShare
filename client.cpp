#include<stdio.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include"protocol.h"


int main(int argc, char const* argv[]){
    char *mode = "lo";
    int client_socket = conect_raw_socket(mode);
    init_protocol(CLIENT, client_socket, 0, 2);

    ifstream input_file ("teste.txt", ios::in|ios::binary|ios::ate);
    char *param_str = "cp_test.txt";
    vector<uint8_t> param = charToVector(param_str, 11);

    send_message(PUT_TYPE, input_file, param);

    close(client_socket);
    return 0;
}
