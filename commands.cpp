#include "commands.h"

fstream null_file;

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
    return exitcode;
}

/************************/
/*      CD              */
/************************/

void cd_client(string directory, int attempts) {
    if(attempts > MAX_ATEMPTS) {
        cout << "Não foi possível enviar o comando\n";
    }
    
    // Envia menssagem
    int result = send_message(CD_TYPE, null_file, directory);

    if(result == MESSAGE_SENT){
        cout << "Sucesso" << endl;
    }
}

void cd_server(string& directory, fs::path& current_path) {
    // Processa string
    fs::path n_path = current_path;
    n_path.append(directory);

    // TODO: Adicionar tratamento para caminho absoluto
    string n_path_str = n_path.generic_u8string();
    char n_path_char[n_path_str.length() + 1];
    strcpy(n_path_char, n_path_str.c_str());

    // TODO: Executar outro comando pra saber se o usuario tem permissao
    
    cout << "CD: verificando " << n_path_char << "\n";
    // Verifica se o diretorio existe
    string cmd = "cd " + n_path_str;
    string result;

    int result_code = execute_command(cmd.c_str(), result);
    if(result_code == 0){
        send_message(OK_TYPE, null_file);
        current_path.append(directory);
    } else {
        // TODO: Adicionar opções de erro
        send_message(ERROR_TYPE, null_file, "A");
    }
}

/************************/
/*      LS              */
/************************/

void ls_client(string parameter) {    
    // Envia menssagem
    send_message(LS_TYPE, null_file, parameter);
}

void ls_server(string parameter, fs::path& current_path) {
    
    string cmd = "ls " + current_path.generic_u8string() + " > server.temp";
    string result;

    int result_code = execute_command(cmd.c_str(), result);

    send_message(OK_TYPE, null_file);

    // Abre o arquivo
    fstream ls_result;
    ls_result.open("server.temp",  ios_base::in | ios_base::out | ios::binary | ios::ate);
    
    cout << "Arquivo criado: " << ls_result.is_open() << "\n";
    send_message(LS_TYPE, ls_result);
}

/************************/
/*      PUT             */
/************************/

void put_client(string parameter, fs::path& current_path) {
    // TODO: Adicionar verificação se arquivo existe

    // Abre arquivo
    string path_to_file = current_path.generic_u8string() + "/" + parameter;
    fstream put_file;
    put_file.open(path_to_file,  ios_base::in | ios_base::out | ios::binary | ios::ate);

    // Envia comando com nome do arquivo
    send_message(PUT_TYPE, put_file, parameter);
}

void put_server(string parameter, fs::path& current_path) {
    string file_path = current_path.generic_u8string() + "/" + parameter;

    // Cria arquivo com mesmo nome
    string cmd = "rm " + file_path + " 2> /dev/null && touch " + file_path;
    string result;
    int result_code = execute_command(cmd.c_str(), result);

    // TODO: Adicionar verificação 
    fstream put_file;
    put_file.open(file_path,  ios_base::in | ios_base::out | ios::binary | ios::ate);   

    send_message(OK_TYPE, null_file);

    // Recebe arquivo
    cout << "Recebendo arquivo" << endl;
    receive_file(PUT_TYPE, put_file);
    cout << "Arquivo recebido" << endl;

}