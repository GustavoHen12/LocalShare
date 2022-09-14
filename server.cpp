#include <stdlib.h>
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include "commands.h"

fs::path PWD = "";

using namespace std;

void wait_input(int *command, string &param_a, string &param_b);

int main(int argc, char const* argv[]){
    string adress = "eno1";
    #ifdef LO_MODE
        adress = "lo";
    #endif
    int server_socket = conect_raw_socket(adress.c_str());
    init_protocol(SERVER, server_socket, 7, 0);

    PWD = fs::current_path();
    
    int command;
    string param_a, param_b;
    while(1){
        // Espera comando de entrada
        wait_input(&command, param_a, param_b);
        cout << command << " - " << param_a << endl;
        switch (command) {
            case CMD_CD:
                cd_server(param_a, PWD);
                cout << "PWD: " << PWD << endl;
                break;
            case CMD_LS:
                ls_server(param_a, PWD);
                cout << "Finalizado: " << PWD << endl;
                break;
            case CMD_PUT:
                put_server(param_a, PWD);
                cout << "Put finalizado: " << PWD << endl;
                break;
            case CMD_GET:
                get_server(param_a, PWD);
                cout << "Get finalizado: " << PWD << endl;
                break;
            case CMD_MKDIR:
                mkdir_server(param_a, PWD);
                cout << "Mkdir finalizado: " << PWD << endl;
            default:
                break;
        }
    }

    shutdown(server_socket, SHUT_RDWR);
    return 0;
}

void wait_input(int *command, string &param_a, string &param_b) {
    msg_t *msg = get_message();
    if(msg == NULL){
        *command = -1;
        return;
    }

    if(msg->type == CD_TYPE){
        *command = CMD_CD;
        param_a = vectorToString(msg->data_bytes, msg->size);
    } else if(msg->type == LS_TYPE) {
        *command = CMD_LS;
        param_a = vectorToString(msg->data_bytes, msg->size);
    } else if(msg->type == PUT_TYPE) {
        *command = CMD_PUT;
        param_a = vectorToString(msg->data_bytes, msg->size);
    } else if(msg->type == GET_TYPE) {
        *command = CMD_GET;
        param_a = vectorToString(msg->data_bytes, msg->size);
    } else if(msg->type == MKDIR_TYPE) {
        *command = CMD_MKDIR;
        param_a = vectorToString(msg->data_bytes, msg->size);
    } else {
        *command = -1;
    }

    // TODO: Implementar demais leituras aqui
}