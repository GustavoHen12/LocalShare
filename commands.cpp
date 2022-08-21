#include "commands.h"

ifstream null_file;

int getCommandCode(string command) {
    if(command == "cd"){
        return CMD_CD;    
    } else if(command == "ls"){
        return CMD_LS;    
    } else if(command == "mkdir"){
        return CMD_MKDIR;    
    } else if(command == "get"){
        return CMD_GET;    
    } else if(command == "put"){
        return CMD_PUT;    
    }

    return -1;
}

void cd_client(string directory, int attempts) {
    if(attempts > MAX_ATEMPTS) {
        cout << "Não foi possível enviar o comando\n";
    }
    
    // Envia menssagem
    send_message(CD_TYPE, null_file, directory);

    // Verifica se a resposta é positiva
    msg_t *response = get_message();
    if(response != NULL && response->type == OK_TYPE){
        cout << "Sucesso" << endl;
        return;
    } else if(response != NULL && response->type == NACK_TYPE){
        cd_client(directory, attempts + 1);
    } else if (response != NULL && response->type == ERROR_TYPE){
        print_error(response);
    }
}

void cd_server(string directory, int attempts) {
    cout << "cd" << endl;
    string nada = "ok";
    send_message(OK_TYPE, null_file, nada);
}