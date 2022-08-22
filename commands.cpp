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

int execute_command(const char* cmd, string &output) {
    FILE *fpipe;
    char c = 0;

    if (0 == (fpipe = (FILE*)popen(cmd, "r"))) {
        cerr << "Não foi possível executar o comando: " << cmd << endl;
    }

    while (fread(&c, sizeof c, 1, fpipe)) output += c;

    int exitcode = pclose(fpipe);
    cout << "Exit code: " << exitcode;
    return exitcode;
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

void cd_server(string& directory, fs::path& current_path) {
    // Processa string
    fs::path n_path = current_path;
    n_path.append(directory);

    string n_path_str = n_path.generic_u8string();
    char n_path_char[n_path_str.length() + 1];
    strcpy(n_path_char, n_path_str.c_str());
    
    cout << "CD: verificando " << n_path_char << "\n";
    // Verifica se o diretorio existe
    string cmd = "ls " + n_path_str;
    string result;

    int result_code = execute_command(cmd.c_str(), result);
    if(result_code == 0){
        send_message(OK_TYPE, null_file, "");
    }

    // TODO: Adicionar opções de erro
    send_message(ERROR_TYPE, null_file, "A");
}