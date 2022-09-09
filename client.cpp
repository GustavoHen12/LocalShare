#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include"commands.h"
fs::path PWD = "";

void read_input_user(int *command, string &param_a, string &param_b);

int main(int argc, char const* argv[]){
    int client_socket = conect_raw_socket("lo");
    init_protocol(CLIENT, client_socket, 0, 7);

    PWD = fs::current_path();

    cout << "************ FTP - Redes 1 ************\n";
    cout << "Bem vindo novamente ao FTP !!!\n";
    cout << "Comandos: \n";
    cout << "\t > cd [DIR]\n";
    cout << "\t > ls [-a] [-l]\n";
    cout << "\t > mkdir [DIR]\n";
    cout << "\t > get [FILE NAME]\n";
    cout << "\t > put [FILE NAME]\n";
    cout << "\t > killsystem\n";
    cout << "Digite o comando após a \">\"\n";
    cout << "**************************************\n";

    int command;
    string param_a, param_b;
    while(1){
        // Le entrada do comando
        read_input_user(&command, param_a, param_b);
        switch (command) {
            case CMD_CD:
                cd_client(param_a);
                break;
            case CMD_LS:
                ls_client(param_a);
                break;
            case CMD_PUT:
                put_client(param_a, PWD);
                break;
            case CMD_GET:
                get_client(param_a, PWD);
                break;
            default:
                break;
        }
    }

    close(client_socket);
    return 0;
}

void read_input_user(int *command, string &param_a, string &param_b){
    cout << "> ";

    string line_str;
    getline(cin, line_str);

    int pos = 0;
    int index = 0;
    while((pos = line_str.find(" ")) != string::npos){
        string op = line_str.substr(0, pos);
        if(index == 0){
            *command = getCommandCode(op);

            // Verifica se o comando é válido
            if(*command == -1) {
                cout << "Comando não encontrado." << endl;
                return;
            }
        } else if (index == 1) {
            param_a = op;
        }

        line_str.erase(0, pos + 1);
        index++;
    }

    if(line_str.size() > 0 && index == 1){
        param_a = line_str;
        index++;
    } else {
        param_b = line_str;
        index++;
    }

    // Verifica se o número de parametros esta correto
    if(index == 1 && *command != CMD_LS){
        cout << "Número inválido de parametros para esse comando" << endl;
        *command = -1;
        return;
    }
}
