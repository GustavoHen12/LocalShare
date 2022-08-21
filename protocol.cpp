#include"protocol.h"

#ifdef DEBUG_PROTOCOL
#define debug_cout(str) do { std::cout << str << std::endl; } while( false )
#else
#define debug_cout(str) do { } while ( false )
#endif

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

vector<uint8_t> charToVector(char *data, int size){
    vector<uint8_t> dt(size);
    for(int i = 0; i < size; i++){
        dt[i] = data[i];
    }

    return dt;
}

int incrementSequence(int sequence){
    int max_sequence = 15; // 2^4 -1
    if(sequence >= max_sequence){
        return 0;
    }
    return sequence+1;
}

/*
* Função recebe como parametro uma mensagem e envia ela para o socket
*/
void send_socket(msg_t *msg){
    int max_size = msg->size + 5 + MSG_MIN_SIZE;
    uint8_t msg_bytes[max_size];

    int msg_bytes_size = 0;

    // Marcador de inicio
    msg_bytes[0] = START_MARKER;

    // Tamanho ocupando 6bits
    msg_bytes[1] = (msg->size & 0b00111111) << 2;

    // Sequencia 4bis = 2bits anterior e 2 depois
    msg_bytes[1] = msg_bytes[1] | (msg->sequence & 0b00001100);
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
    
    debug_cout("Enviando mensagem " << msg->sequence << "...");
    send(app_info.socket, msg_bytes, msg_bytes_size, 0); 
    debug_cout("Enviado");
    
    // Incrementa sequencia
    app_info.last_type = msg->type;
    app_info.sequence = incrementSequence(app_info.sequence);
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
    for (int i = 0; i < data_size; i++){
        // Verifica se o buffer continua valido
        if(i + 3 >= buffer_size){
            return NULL;
        }

        data.push_back(buffer[i + 3]);
        parity^=buffer[i + 3];
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

    // TODO: Verifica paridade
    if(msg->parity != parity){
        debug_cout("Erro paridades não são iguais");
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
        msg->parity = parity;
    } else {
        msg->size = 0;
    }

    debug_cout("Mensagem criada: ");
    printMessage(msg, "\t");
    return msg;
}

/************ FUNÇÕES DO PROTOCOLO ************/
void init_protocol(int type, int socket, int sequence, int target_sequence){
    app_info.sequence = sequence;
    app_info.type = type;
    app_info.target_sequence = target_sequence;
    app_info.socket = socket;
}

msg_t *get_message(){
    // TODO: Implementar timeout
    debug_cout("Esperando mensagem");
    do{
        vector<uint8_t> buf(100);
        int bytes = recv(app_info.socket, buf.data(), buf.size() - 1, 0);
        debug_cout("Recebido");

        if(bytes < MSG_MIN_SIZE || bytes > MSG_MAX_SIZE){
            continue;
        }
        
        msg_t *msg = bytesToMessage(buf, bytes);
        
        // Verifica se a mensagem que foi recebida é a esperada
        if(msg != NULL && (msg->sequence == app_info.target_sequence)){
            app_info.target_sequence = incrementSequence(app_info.target_sequence);
            debug_cout("Valida");
            printMessage(msg, "\t");
            return msg;
        } else {
            int seq = msg->sequence;
            debug_cout("Invalida. Esperado: " << app_info.target_sequence << " / " << seq << endl);
        }
    } while(1);
    
    return NULL;
}

int send_message(uint8_t type, ifstream& data, vector<uint8_t>& param) {
    // monta mensagem inicial
    msg_t *start_msg = new_message(type, param);
    if(start_msg == NULL){
        cerr << "Não foi possível criar a menssagem !\n";
        return 0;
    }

    // Envia mensagem inicial e verifica resposta
    send_socket(start_msg);
    sleep(5);
    // msg_t *resolve = get_message();
    // printMessage(resolve);

    if(data.is_open()) {
        // Verifica tamanho do arquivo
        streampos size = data.tellg();
        long long fileSize = size;

        // Manda tamanho do arquivo se necessário
        if((type == PUT_TYPE && app_info.type == CLIENT) || (type == GET_TYPE && app_info.type == SERVER)){
            vector<uint8_t> size_f;
            size_f.push_back(fileSize);
            msg_t *size_msg = new_message(SIZEF_TYPE, size_f);
            send_socket(size_msg);
            // msg_t *resolve = get_message();
            // printMessage(resolve);
        }
        
        sleep(5);
        
        // Coloca cursor no inicio do arquivo
        data.seekg (0, ios::beg);
        
        // Envia dados
        long long to_send = fileSize;
        cout << "File size: " << size << "\n";
        char *datablock;
        while(to_send > 0){
            // Manda 4 pacotes de uma vez
            for(int i = 0; i < 4; i++){
                // Pega tamanho em bytes do bloco a ser enviado
                int bytes = to_send < DATA_SIZE_BYTES ? to_send : DATA_SIZE_BYTES;
                datablock = new char[bytes];
                
                // Le o bloco
                data.read (datablock, bytes);

                // Converte para vetor e envia
                vector<uint8_t> vec_data = charToVector(datablock, bytes);
                msg_t *data_msg = new_message(DATA_TYPE, vec_data);
                send_socket(data_msg);
                
                // Decrementa e verificar se já acabou
                to_send -= bytes;
                if(to_send <= 0)
                    break;
            }

            // msg_t *answer = get_message();
            // if(answer->type != OK_TYPE){
            //     break;
            // }
        }

        // Tem que receber algo ?
            // Recebe e trata 
    }

    return 1;
}