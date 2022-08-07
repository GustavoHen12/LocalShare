#include"protocol.h"

void init_protocol(int type){
    app_info.sequence = 0;
    app_info.type = type;
}

vector<uint8_t> charToVector(char *data, int size){
    vector<uint8_t> dt(size);
    for(int i = 0; i < size; i++){
        dt[i] = data[i];
    }

    return dt;
}

void send_socket(msg_t *msg){
    int max_size = msg->msg_size +  5;
    uint8_t msg_bytes[max_size];

    int msg_bytes_size = 0;

    // Marcador de inicio
    msg_bytes[0] = START_MARKER;

    // Tamanho ocupando 6bits
    msg_bytes[1] = (msg->msg_size & 0b00111111) << 2;

    // Sequencia 4bis = 2bits anterior e 2 depois
    msg_bytes[1] = msg_bytes[1] | (msg->sequence & 0b00001100);
    msg_bytes[2] = (msg->sequence & 0b00000011) << 6;

    // Typo 6bits = Todos na anterior
    msg_bytes[2] = msg_bytes[2] | (msg->type & 0b00111111);

    // Copia dados
    msg_bytes_size = 3;
    for(int i = 0; i < msg->msg_size; i++){
        msg_bytes[i] = msg->data_bytes[i];
        msg_bytes_size++;
    }
    
    // Paridade ultimos 8 bits
    msg_bytes[msg_bytes_size] = msg->parity;
    msg_bytes_size++;
    
    send(app_info.socket, msg_bytes, msg_bytes_size, 0);
}

int wait_answer(){
    return 0;
}

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
        msg->msg_size = (data.size() & 0b00111111);
        
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
        msg->msg_size = 0;
    }
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
    int resolve = wait_answer();

    if(data.is_open()) {
        // Verifica tamanho do arquivo
        streampos size = data.tellg();
        long long fileSize = size;

        // precisa mandar tamanho do arquivo ?
        if((type == PUT_TYPE && app_info.type == CLIENT) || (type == GET_TYPE && app_info.type == SERVER)){
            // manda tamanho arquivo
            send_socket(new_message(SIZEF_TYPE, param));
            int resolve = wait_answer();
        }
        
        // Coloca cursor no inicio do arquivo
        data.seekg (0, ios::beg);

        // Enquanto não chegou ao fim
        long long to_send = fileSize;
        char *datablock;
        while(to_send > 0){
            // Manda 4 de uma vez
            for(int i = 0; i < 4; i++){
                int bytes = to_send < DATA_SIZE_BYTES ? to_send : DATA_SIZE_BYTES;
                datablock = new char[bytes];
                data.read (datablock, bytes);

                send_socket(new_message(DATA_TYPE, charToVector(datablock, bytes)));
                to_send -= bytes;
            }


        }

        // Tem que receber algo ?
            // Recebe e trata 
    }

    return 1;
}