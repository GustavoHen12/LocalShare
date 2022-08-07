#include"protocol.h"

/************ UTIL ************/
vector<uint8_t> charToVector(char *data, int size){
    vector<uint8_t> dt(size);
    for(int i = 0; i < size; i++){
        dt[i] = data[i];
    }

    return dt;
}

int incrementSequence(int sequence){
    int max_sequence = 15; // 2^4 -1
    if(sequence >= max_sequence)
        return 0;
    return sequence++;
}

/*
* Função recebe como parametro uma mensagem e envia ela para o socket
*/
void send_socket(msg_t *msg){
    int max_size = msg->size +  5;
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
        msg_bytes[i] = msg->data_bytes[i];
        msg_bytes_size++;
    }
    
    // Paridade ultimos 8 bits
    msg_bytes[msg_bytes_size] = msg->parity;
    msg_bytes_size++;
    
    send(app_info.socket, msg_bytes, msg_bytes_size, 0);
    
    // Incrementa sequencia
    app_info.sequence = incrementSequence(app_info.sequence);
}

/*
* A partir do tipo e do vetor de dados cria uma nova mensagem
*/

//TODO: Arrumar calculo de paridade
msg_t * new_message(uint8_t type, vector<uint8_t> &data) {
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
        
        int total_1 = 0;
        // Calcula paridade
        for(auto &bt : data){
            for(int i = 0; i < 8; i++){
                if(bt & (1 << i) != 0)
                    total_1++;
            }
        }
        msg->data_bytes = data;
    } else {
        msg->size = 0;
    }

    return msg;
}

msg_t *bytesToMessage(vector<uint8_t> buffer, int size){
    msg_t *msg;
    msg = (msg_t*) calloc(1, sizeof(msg_t));
    if(!msg){
        return NULL;
    }

    // Marcador de inicio: 8bits
    uint8_t startMarker = buffer[0];
    if(startMarker != START_MARKER)
        return NULL;

    // Tamanho da mensagem: 6bits
    msg->size = (buffer[1] & 0b11111100) >> 2;

    msg->sequence = (buffer[1] & 0b00000011) << 2;
    msg->sequence = msg->sequence | ((buffer[2] & 0b11000000) >> 6);

    // Tipo: 6bits
    msg->type = (buffer[2] & 0b00111111);

    // Dados
    int data_size = 0 | msg->size;
    vector<uint8_t> data;
    for (int i = 0; i < data_size; i++){
        data.push_back(buffer[i + 2]);
    }
    msg->data_bytes = data;

    // Paridade: 8bits
    msg->parity = buffer[data_size + 2];

    // TODO: Verifica paridade
    return msg;
}

/************ FUNÇÕES DO PROTOCOLO ************/
void init_protocol(int type){
    app_info.sequence = 0;
    app_info.type = type;
}

msg_t *get_message(){
    // TODO: Implementar timeout
    do{
        vector<uint8_t> buf(5000);
        int bytes = recv(app_info.socket, buf.data(), buf.size(), 0);
        msg_t *msg = bytesToMessage(buf, bytes);
        
        // Verifica se a mensagem que foi recebida é a esperada
        if(msg != NULL && (msg->sequence == app_info.target_sequence)){
            app_info.target_sequence = incrementSequence(app_info.target_sequence);
            return msg;
        }
    } while(1);
    
    return NULL;
}

int send_message(uint8_t type, ifstream data, vector<uint8_t>& param, int origin) {
    // monta mensagem inicial
    msg_t *start_msg = new_message(type, param);
    if(start_msg == NULL){
        cerr << "Não foi possível criar a menssagem !\n";
        return 0;
    }

    // Envia mensagem inicial e verifica resposta
    send_socket(start_msg);
    msg_t *resolve = get_message();

    if(data.is_open()) {
        // Verifica tamanho do arquivo
        streampos size = data.tellg();
        long long fileSize = size;

        // Manda tamanho do arquivo se necessário
        if((type == PUT_TYPE && app_info.type == CLIENT) || (type == GET_TYPE && app_info.type == SERVER)){
            send_socket(new_message(SIZEF_TYPE, param));
            msg_t *resolve = get_message();
        }
        
        // Coloca cursor no inicio do arquivo
        data.seekg (0, ios::beg);
        
        // Envia dados
        long long to_send = fileSize;
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
                send_socket(new_message(DATA_TYPE, vec_data));
                
                // Decrementa e verificar se já acabou
                to_send -= bytes;
                if(to_send <= 0)
                    break;
            }

            msg_t *answer = get_message();
            if(answer->type != OK_TYPE){
                break;
            }
        }

        // Tem que receber algo ?
            // Recebe e trata 
    }

    return 1;
}