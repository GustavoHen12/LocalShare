#include "commands.h"

fstream null_file;

int DONT_HAVE_PERMISSION = -1, DONT_EXISTS=-2, EXISTS=2, HAVE_PERMISSION=1;

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

int verify_permission(string path) {
    // Verifica se tem permissao
    string cmd = "stat -c \"%a\" " + path;
    string result;
    int result_code = execute_command(cmd.c_str(), result);
    cout << cmd << " - " << result << endl;
    if(result[0] != '7'){
        cout << "Sem permissão" << endl;
        return DONT_HAVE_PERMISSION;
    }

    return HAVE_PERMISSION;
}

int verify_if_exist(string path, bool isDir=true) {
    string op = isDir ? "d" : "f";
    string cmd = "test -" + op + " " + path +" && echo \"1\"";
    cout << cmd << endl;
    string result;
    int result_code = execute_command(cmd.c_str(), result);
    if(result_code == 0 && result.size() > 0 && result[0] == '1'){
        return EXISTS;
    }

    cout << "Não existe" << endl;
    return DONT_EXISTS;
}

int append_path(string new_path, fs::path& current_path) {
    if(new_path[0] == '.'){
        current_path.append(new_path);
    } else {
        current_path.assign(new_path);
    }

    return 1;
}

/************************/
/*      CD              */
/************************/

void cd_client(string directory, fs::path& current_path, fs::path& server_path, bool server_cmd) {
    // Envia menssagem
    int result = send_message(CD_TYPE, null_file, directory);

    if(result == MESSAGE_SENT){
        append_path(directory, server_path);
    }
}

void cd_server(string& directory, fs::path& current_path) {
    // Processa string
    fs::path n_path = current_path;
    append_path(directory, n_path);

    // Verifica se o diretório existe
    if(verify_if_exist(n_path.generic_u8string()) != EXISTS){
        send_message(ERROR_TYPE, null_file, DIR_DONT_EXISTS);
        return;
    }

    // Verifica se o usuário tem permissão
    if(verify_permission(n_path.generic_u8string()) != HAVE_PERMISSION){
        send_message(ERROR_TYPE, null_file, ERROR_PERMISSION);
        return;
    }

    string cmd = "cd " + n_path.generic_u8string();
    string result;
    int result_code = execute_command(cmd.c_str(), result);
    if(result_code == 0){
        send_message(OK_TYPE, null_file);
        append_path(directory, current_path);
    } else {
        send_message(ERROR_TYPE, null_file, DIR_DONT_EXISTS);
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
    // Verificação se arquivo existe
    fs::path n_path = current_path;
    append_path(parameter, n_path);

    if(verify_if_exist(n_path, false) == DONT_EXISTS) {
        cout << "Arquivo não existe" << endl;
        return;
    }

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
    string cmd = "rm -f " + file_path + " && touch " + file_path;
    string result;
    int result_code = execute_command(cmd.c_str(), result);

    fstream put_file;
    put_file.open(file_path,  ios_base::in | ios_base::out | ios::binary | ios::ate);   

    send_message(OK_TYPE, null_file);

    // Recebe arquivo
    cout << "Recebendo arquivo" << endl;
    receive_file(PUT_TYPE, put_file);
    cout << "Arquivo recebido" << endl;
}

/************************/
/*      GET             */
/************************/
void get_client(string parameter, fs::path& current_path) {
    string file_path = current_path.generic_u8string() + "/" + parameter;
    cout << file_path << endl;
    // Cria arquivo com mesmo nome
    string cmd = "rm -f " + file_path + " && touch " + file_path;
    string result;
    int result_code = execute_command(cmd.c_str(), result);

    cout << cmd << endl;
    cout << result_code << endl;

    fstream get_file;
    get_file.open(file_path,  ios_base::in | ios_base::out | ios::binary | ios::ate);   

    send_message(GET_TYPE, get_file, parameter);
}

void get_server(string parameter, fs::path& current_path) {
    // Verificação se arquivo existe
    fs::path n_path = current_path;
    append_path(parameter, n_path);

    if(verify_if_exist(n_path, false) == DONT_EXISTS) {
        send_message(ERROR_TYPE, null_file, FILE_DONT_EXISTS);
        return;
    }

    // Abre arquivo
    string path_to_file = current_path.generic_u8string() + "/" + parameter;
    fstream get_file;
    get_file.open(path_to_file,  ios_base::in | ios_base::out | ios::binary | ios::ate);

    // Envia comando com nome do arquivo
    send_message(GET_TYPE, get_file, parameter);
}
