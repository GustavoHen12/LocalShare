#include"protocol.h"

#ifdef DEBUG_PROTOCOL
#define debug_cout(str) do { std::cout << str << std::endl; } while( false )
#else
#define debug_cout(str) do { } while ( false )
#endif

vector<uint8_t> problem_bytes {(uint8_t) 126, (uint8_t) 136};

app_info_t app_info;

/************ UTIL ************/
void printMessage(msg_t *msg, string prefix){
    cout << prefix << "Tamanho: " << (int) msg->size << " / " << bitset<8>(msg->size) << endl;    
    cout << prefix << "Sequencia: " << (int) msg->sequence << " / " << bitset<8>(msg->sequence) << endl;    
    cout << prefix << "Tipo: " << (int) msg->type << " / " << bitset<8>(msg->type) << endl;
    cout << prefix << "Dados: ";
    for (int i = 0; i < (int) msg->size; i++){
        cout << (char)(msg->data_bytes)[i] << " | ";
    }
    cout << endl;
}

bool problemByte(uint8_t b1){
    return find(problem_bytes.begin(), problem_bytes.end(), b1) != problem_bytes.end();
}

vector<uint8_t> charToVector(char *data, int size){
    vector<uint8_t> dt(size);
    for(int i = 0; i < size; i++){
        dt[i] = data[i];
    }

    return dt;
}

vector<uint8_t> stringToVector(string str){
    vector<uint8_t> dt(str.size());
    for(int i = 0; i < str.size(); i++){
        dt[i] = (char) str[i];
    }

    return dt;
}

string vectorToString(vector<uint8_t> &data, int size) {
    string str;
    for(int i = 0; i < size; i++) {
        str += (char) data[i];
    }

    return str;
}

int incrementSequence(int sequence, bool targetSequence=false){
    int min_sequence = 0;
    int max_sequence = 15; // 2^4 -1

#ifdef LO_MODE
    if(app_info.type == CLIENT) {
        max_sequence = targetSequence ? 15 : 7;
        min_sequence = targetSequence ? 8 : 0;
    } else {
        max_sequence = targetSequence ? 7 : 15;
        min_sequence = targetSequence ? 0 : 8;
    }
#endif

    if(sequence >= max_sequence){
        return min_sequence;
    }

    return sequence + 1;
}

int decrementSequence(int sequence, bool targetSequence=false){
    int min_sequence = 0;
    int max_sequence = 15; // 2^4 -1

#ifdef LO_MODE
    if(app_info.type == CLIENT) {
        max_sequence = targetSequence ? 15 : 7;
        min_sequence = targetSequence ? 8 : 0;
    } else {
        max_sequence = targetSequence ? 7 : 15;
        min_sequence = targetSequence ? 0 : 8;
    }
#endif

    if(sequence <= min_sequence){
        return max_sequence;
    }

    return sequence - 1;
}

int processResponse(msg_t *response){
    if((response->type == ACK_TYPE) || (response->type == OK_TYPE)){
        return MESSAGE_SENT;
    } else if (response->type == NACK_TYPE) {
        return RESEND;
    } else if (response->type == ERROR_TYPE) {
        if(response->data_bytes.size() > 0) {
            string error = "";
            error += ((char) response->data_bytes[0]);
            if(error == DIR_DONT_EXISTS){
                cout << "Erro: diretório não existe !" << endl; 
            } else if (error == DIR_ALREADY_EXISTS) {
                cout << "Erro: diretório já existe !" << endl; 
            } else if (error == ERROR_PERMISSION) {
                cout << "Erro: você não tem permissão !" << endl; 
            } else if (error == FILE_DONT_EXISTS) {
                cout << "Erro: arquivo não existe !" << endl; 
            }
        } else {
            cout << "Erro não identificado !!!" << endl;
        }

        return ERROR;
    }

    return 0;
}

msg_t *bytesToMessage(vector<uint8_t> buffer, int buffer_size){
    msg_t *msg;
    msg = (msg_t*) calloc(1, sizeof(msg_t));
    if(!msg){
        return NULL;
    }

    // Marcador de inicio: 8bits
    uint8_t startMarker = buffer[0];
    if(startMarker != START_MARKER)
        return NULL;
    
    debug_cout("Lendo Mensagem ..." << bitset<8>(startMarker));

    // Tamanho da mensagem: 6bits
    msg->size = (buffer[1] & 0b11111100) >> 2;

    msg->sequence = (buffer[1] & 0b00000011) << 2;
    msg->sequence = msg->sequence | ((buffer[2] & 0b11000000) >> 6);

    // Tipo: 6bits
    msg->type = (buffer[2] & 0b00111111);

    // Dados
    int data_size = 0 | msg->size;
    vector<uint8_t> data;
    uint8_t parity = 0;
    uint8_t last_byte = 0;
    for (int i = 0; i < data_size; i++){
        // Verifica se o buffer continua valido
        if(i + 3 >= buffer_size){
            return NULL;
        }
        
        /*
        if(!problemByte(last_byte)){
            data.push_back((uint8_t) buffer[i + 3]);
        } else {
            msg->size--;
        }
        last_byte = buffer[i + 3];*/
        
        data.push_back((uint8_t) buffer[i + 3]);
        parity^= (uint8_t) buffer[i + 3];
    }
    msg->data_bytes = data;

    // Complemento
    int offset = 0;
    if(msg->size < MSG_MIN_SIZE){
        offset = MSG_MIN_SIZE - msg->size; 
    }

    // Verifica se o buffer continua valido
    if(data_size + offset + 3 >= buffer_size){
        return NULL;
    }

    // Paridade: 8bits
    // o inicio é depois do:
    // dados iniciais(3 bytes) + tamanho da area de dados + complemento
    msg->parity = buffer[data_size + offset + 3];

    if(msg->parity != parity){
        cout << "Erro paridades não são iguais" << endl;
        // Envia nack
        fstream null_file;
        string param = "" + ((int) msg->parity + '0');
        send_message(NACK_TYPE, null_file, param);
        return NULL;
    }
    return msg;
}

/* A partir do tipo e do vetor de dados cria uma nova mensagem */
msg_t *new_message(uint8_t type, vector<uint8_t> &data) {
    debug_cout("Criando mensagem...");

    // aloca espaço para mensagem
    msg_t *msg;
    msg = (msg_t*) calloc(1, sizeof(msg_t));
    if(!msg){
        return NULL;
    }

    // Inicia os campos
    msg->type = (type & 0b00111111);
    msg->sequence = (app_info.sequence & 0b00001111);

    if(data.size() > 0){
        msg->size = (data.size() & 0b00111111);

        msg->data_bytes = data;

        // Calcula paridade
        uint8_t parity = 0;
        for(auto &bt : data){
            parity ^= bt;
        }

        /*
        vector<uint8_t> data_msg;
        int size = data.size();
        int i = 0;
        while(i < size) {
            data_msg.push_back(data[i]);
            if(problemByte(data[i])){
                i++;
                data_msg.push_back(0xf);
                size++;
            }
            i++;
        }
        msg->data_bytes = data_msg;
        msg->size = (size & 0b00111111);

        uint8_t parity = 0;
        for(auto &bt : data_msg){
            parity ^= bt;
        } */
       
        msg->parity = parity;
    } else {
        msg->size = 0;
    }

    debug_cout("Mensagem criada: ");
    // printMessage(msg, "\t");
    return msg;
}

/************ FUNÇÕES DO PROTOCOLO ************/
void init_protocol(int type, int socket, int sequence, int target_sequence){
    app_info.sequence = sequence;
    app_info.type = type;
    app_info.target_sequence = target_sequence;
    app_info.socket = socket;

#ifdef LO_MODE
    if(app_info.type == CLIENT) {
        app_info.sequence = 0;
        app_info.target_sequence = 8;
    } else {
        app_info.sequence = 8;
        app_info.target_sequence = 0;
    }
#endif

    app_info.last_msg_replied = -1;
}

/*
* Função recebe como parametro uma mensagem e envia ela para o socket
*/
void raw_send(msg_t *msg){
    int max_size = msg->size + 5 + MSG_MIN_SIZE;
    uint8_t msg_bytes[max_size];

    int msg_bytes_size = 0;

    // Marcador de inicio
    msg_bytes[0] = START_MARKER;

    // Tamanho ocupando 6bits
    msg_bytes[1] = (msg->size & 0b00111111) << 2;

    // Sequencia 4bis = 2bits anterior e 2 depois
    uint8_t temp_seq = (msg->sequence & 0b00001100);
    msg_bytes[1] = msg_bytes[1] | (temp_seq >> 2);
    msg_bytes[2] = (msg->sequence & 0b00000011) << 6;

    // Typo 6bits = Todos na anterior
    msg_bytes[2] = msg_bytes[2] | (msg->type & 0b00111111);

    // Copia dados
    msg_bytes_size = 3;
    for(int i = 0; i < msg->size; i++){
        msg_bytes[i + 3] = msg->data_bytes[i];
        msg_bytes_size++;
    }

    // Complemento
    if(msg->size < MSG_MIN_SIZE){
        for(int i = msg->size; i < MSG_MIN_SIZE; i++){
            msg_bytes[msg_bytes_size] = 'c';
            msg_bytes_size++;
        }
    }
    
    // Paridade ultimos 8 bits
    msg_bytes[msg_bytes_size] = msg->parity;
    msg_bytes_size++;
    
    send(app_info.socket, msg_bytes, msg_bytes_size, 0); 
    
    // Salva mensagem
    app_info.last_msg = msg;
    app_info.last_msg_replied = decrementSequence(app_info.target_sequence, true);
    app_info.can_retry = 1; 
}

void send_socket(msg_t *msg){
    raw_send(msg);
    // Incrementa sequencia
    app_info.sequence = incrementSequence(app_info.sequence);
}

msg_t *get_message(){
    do{
        vector<uint8_t> buf(300);
        int bytes = recv(app_info.socket, buf.data(), buf.size() - 1, 0);

        if(bytes == -1) { // Ocorreu um timeout
            // Cria uma mensagem "falsa" como um nack
            vector<uint8_t> param = stringToVector("");
            msg_t *nack_msg = new_message(NACK_TYPE, param);
            return nack_msg;
        }

        if(bytes < MSG_MIN_SIZE || bytes > MSG_MAX_SIZE){
            continue;
        }
        
        msg_t *msg = bytesToMessage(buf, bytes);

        // Verifica se a mensagem que foi recebida é a esperada
        if((msg != NULL) && (msg->sequence >= 0)){
            // Verifica se a mensagem é a atual ou a anterior
            if(msg->sequence == app_info.target_sequence){
                // if((int) msg->sequence == (int) decrementSequence(app_info.sequence)){
                //     cout << "echo" << endl;
                //     // continue;
                // }
                // Se for a atual incrementa o contador e retorna a mensagem
                app_info.target_sequence = incrementSequence(app_info.target_sequence, true);
                app_info.last_msg_received = msg->sequence;
                // cout << "\nOK: Recebi: " << (int) msg->sequence << " agora esperava: " << (int) app_info.target_sequence << " ultima env: " << (int)(app_info.last_msg)->sequence << " mi seq: " << (int) app_info.sequence << endl;
                return msg;
            } 
            else if (app_info.last_msg_replied != -1 && app_info.last_msg_received == msg->sequence && msg->sequence == app_info.last_msg_replied && app_info.can_retry > 0) {
                // cout << "Responde ultima mensagem " << (int) msg->sequence << " == " << app_info.last_msg_replied << " esperando: " << (int) app_info.target_sequence;
                // Se for uma anterior, significa que a resposta foi perdida então é reenviada
                raw_send(app_info.last_msg);
                app_info.can_retry--;
                // cout << "Agora esperando: " << (int) app_info.target_sequence << endl;
            }
            // else {
            //     cout << "Recebi: " << (int) msg->sequence << " esperava: " << (int) app_info.target_sequence << " ultima: " << app_info.last_msg_received << " resp: " << app_info.last_msg_replied << endl;
            // }
        }
    } while(1);
    
    return NULL;
}

msg_t* wait_until_valid(msg_t *last_msg){
    int status;
    msg_t *resolve;
    while(1){
        resolve = get_message();
        status = processResponse(resolve);
        if(status == RESEND){
            cout << "Mensagem invalida, vai reenviar" << endl;
            // Como na ultima vez que enviou a sequancia foi incrementada
            // decrementa para enviar com a mesma sequencia
            app_info.sequence = decrementSequence(app_info.sequence);
            send_socket(last_msg);
        } else {
            break;
        }
    }

    return resolve; 
}

int send_file(uint8_t type, fstream& data){
    int status;

    // Verifica tamanho do arquivo
    streampos size = data.tellg();
    long long fileSize = size;

    // Manda tamanho do arquivo se necessário
    if((type == PUT_TYPE && app_info.type == CLIENT) || (type == GET_TYPE && app_info.type == SERVER)){
        cout << "Enviando tamanho do arquivo: " << fileSize << endl;
        vector<uint8_t> size_f;
        for(int i = 0; i < sizeof(long long); i++) {
            uint8_t byte = fileSize >> (8*i);
            size_f.push_back(byte);
        }

        msg_t *size_msg = new_message(SIZEF_TYPE, size_f);

        send_socket(size_msg);
        msg_t *resolve = wait_until_valid(size_msg);

        cout << "Enviado tamanho do arquivo: " << fileSize << endl;
    }
        
    // Coloca cursor no inicio do arquivo
    data.seekg(0, ios::beg);
    
    // Envia dados
    long long to_send = fileSize;
    cout << "File size: " << size << "\n";

    long long bytes_sended_size = 0;
    vector<pair<msg_t *, int>> buffer_send;
    while(to_send > 0 || buffer_send.size() > 0){
        // Preenche o buffer
        int bytes_sended = 0;
        while (to_send > 0 && buffer_send.size() != WINDOW_SIZE) {
            int bytes = to_send < DATA_SIZE_BYTES ? to_send : DATA_SIZE_BYTES;
            char *datablock = new char[bytes];
            
            // Le o bloco
            data.read(datablock, bytes);
            bytes_sended += bytes;

            // Converte para vetor e escolhe o tipo correto
            vector<uint8_t> vec_data = charToVector(datablock, bytes);
            uint8_t data_type = type == LS_TYPE ? PRINT_TYPE : DATA_TYPE;

            // Cria mensagem e envia
            msg_t *data_msg = new_message(data_type, vec_data);
            app_info.sequence = incrementSequence(app_info.sequence);

            buffer_send.push_back(make_pair(data_msg, 0));
            to_send -= bytes;
            bytes_sended_size += bytes;
        }
        
        // Envia
        for(int i = 0; i < buffer_send.size(); i++){
            raw_send(buffer_send[i].first);
            buffer_send[i].second = 1;
        }

        if(buffer_send.size() == WINDOW_SIZE){
            msg_t *answer = get_message();
            int status = processResponse(answer);
            if(status == ERROR){
                cout << "Error" << endl;
                to_send = -1;
                break;
            } else if (status == RESEND){
                // Verifica NACK
                cout << "Resend" << endl;
                int not_sended = WINDOW_SIZE;
                int bytes_not_sended = 0;

                if(answer->size > 0 && answer->data_bytes.size() > 0 && (int) answer->data_bytes[0] < app_info.sequence){
                    for(int i = 0; i < buffer_send.size(); i++) {
                        if(((int) (buffer_send[i].first)->sequence) == (int) app_info.sequence) {
                            not_sended = i - WINDOW_SIZE;
                        } else {
                            buffer_send[i].second = 1;
                        }
                        if(i >= not_sended ){
                            bytes_not_sended += (buffer_send[i].first)->data_bytes.size(); 
                            buffer_send[i].second = 0;
                        }
                    }
                } else {
                    bytes_not_sended = bytes_sended;
                    for(int i = 0; i < buffer_send.size(); i++) {
                        buffer_send[i].second = 0;
                    }
                }
                
                // Aumenta quantas ainda deve enviar
                bytes_sended_size -= bytes_not_sended;                
            } else {
                for(int i = 0; i < buffer_send.size(); i++) {
                    buffer_send[i].second = 1; 
                }
            }
        }

        // Limpa o buffer
        int i = 0;
        while(i < buffer_send.size()){
            if(buffer_send[i].second == 1){
                buffer_send.erase(buffer_send.begin() + i);
            } else {
                i++;
            }
        }
    }

    cout << "Enviei: " << bytes_sended_size << " bytes" << endl;
    vector<uint8_t> nada;
    msg_t *end_msg = new_message(END_TYPE, nada);
    send_socket(end_msg);

    return status;
}

int receive_file(uint8_t type, fstream& data) {
    fstream null_file;
    
    // Recebe tamanho do arquivo
    if((type == PUT_TYPE && app_info.type == SERVER) || (type == GET_TYPE && app_info.type == CLIENT)) {
        cout << "Recendo tamanho do arquivo" << endl;

        msg_t *size_msg = get_message();
        int status = processResponse(size_msg);
        if(status == ERROR)
            return ERROR;
        
        if(size_msg->type == SIZEF_TYPE) {
            vector<uint8_t> size_v = size_msg->data_bytes;
            long long size = 0;
            for(int i = 0; i < sizeof(long long); i++){
                uint8_t byte = size_v[i];
                size = size | (byte << 8*i);
            }
            cout << "Estou recebendo um arquivo de " << size << " bytes " << endl;
            send_message(OK_TYPE, null_file);
        } else {
            cout << "Recebi isso: " << bitset<8>(size_msg->type) << endl;
            send_message(NACK_TYPE, null_file);
        }

    }

    int status;
    int bytes_received = 0;
    // Enquanto não houver erro
    vector<msg_t *> buffer_receive;
    int last_seq = -1;
    do {
        msg_t *msg = get_message();

        // Verfica timmeout (perdeu alguma mensagem da janela)
        if(msg->type == NACK_TYPE) { 
            cout << "Ocorreu erro timeout" << "\n";
            status = RESEND;
        } else if (msg->type == PRINT_TYPE) {                
            for(int i = 0; i < msg->size; i++){
                cout << msg->data_bytes[i];
            }
            status = MESSAGE_SENT;
            buffer_receive.push_back(msg);
        } else if (msg->type == DATA_TYPE) {
            for(int i = 0; i < msg->size; i++){
                data << msg->data_bytes[i];
                bytes_received++;
            }
            status = MESSAGE_SENT;
            buffer_receive.push_back(msg);
        } else if(msg->type == END_TYPE){
            status = END_COMMAND;
            break;
        } else {
            cout << "\nNão reconheci: " << bitset<8>(msg->type) << endl;
            app_info.target_sequence = decrementSequence(app_info.target_sequence, true);
        }

        // for(int i = 0; i < buffer_receive.size(); i++) cout << " " << (int) (buffer_receive[i])->sequence;

        if(buffer_receive.size() == WINDOW_SIZE){
            send_message(ACK_TYPE, null_file);
            if(status == MESSAGE_SENT){
                buffer_receive.clear();
            }
        }

    } while(status != END_COMMAND);

    // cout << "Recebido " << bytes_received << " bytes" << endl;
    return status;
}

bool message_send_data(uint8_t type){
    if(app_info.type == SERVER){
        return type == LS_TYPE || type == GET_TYPE;
    }
    return type == PUT_TYPE;
}

bool message_receive_file(uint8_t type){
    if(app_info.type == SERVER){
        return type == PUT_TYPE;
    }
    return type == GET_TYPE || type == LS_TYPE;
}

bool send_initial_message(uint8_t type) {
    return !((app_info.type == SERVER && type == LS_TYPE) || (app_info.type == SERVER && type == GET_TYPE));
}

bool isResponseMessage(uint8_t type) {
    return type == OK_TYPE || type == NACK_TYPE || type == ACK_TYPE || type == ERROR_TYPE;
}

bool ignoreResponse(uint8_t type){
    return isResponseMessage(type) || (type == GET_TYPE && app_info.type == CLIENT);
}

int send_message(uint8_t type, fstream& data, string param_str) {
    debug_cout("sent_message: Iniciando...");

    // Estado do envio dos dados
    int status;

    // Converte param para vetor de uint8_t 
    vector<uint8_t> param = stringToVector(param_str);

    // Monta mensagem inicial
    msg_t *start_msg = new_message(type, param);
    if(start_msg == NULL){
        cerr << "Não foi possível criar a menssagem !\n";
        return ERROR;
    }

    if(send_initial_message(type)){
        // Envia mensagem inicial
        send_socket(start_msg);

        // Verifica resposta
        if(!ignoreResponse(type)){
            msg_t *resolve = wait_until_valid(start_msg);
            status = processResponse(resolve);
        }
    }

    // Envia dados
    if(message_send_data(type) && data.is_open() && status != ERROR) {
        cout << "Enviando arquivo" << endl;
        status = send_file(type, data);
        cout << "Arquivo enviado" << endl;
    }
    
    if(message_receive_file(type) && status != ERROR) {
        //cout << "Esperando arquivo" << endl;
        status = receive_file(type, data);
    }

    debug_cout("sent_message: Finalizado");
    return status;
}
