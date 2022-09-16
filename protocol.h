#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

#include"socket_utils.h"

#define SERVER 0
#define CLIENT 1

/****** CODIGOS INTERNOS ******/
#define MESSAGE_SENT 1
#define ERROR  -1

#define RESEND  -2
#define END_COMMAND  2

#define WINDOW_SIZE 4

/****** MESSAGE TYPES ******/
#define OK_TYPE     0b000001
#define NACK_TYPE   0b000010
#define ACK_TYPE    0b000011

#define CD_TYPE     0b000110
#define LS_TYPE     0b000111
#define MKDIR_TYPE  0b001000
#define PUT_TYPE    0b001001
#define GET_TYPE    0b001010

#define DATA_TYPE    0b100000
#define ERROR_TYPE   0b010001
#define SIZEF_TYPE   0b011000
#define END_TYPE     0b101110
#define PRINT_TYPE   0b111111

/****** ERROR TYPE******/
#define DIR_DONT_EXISTS    "A"
#define ERROR_PERMISSION   "B"
#define DIR_ALREADY_EXISTS "C"
#define FILE_DONT_EXISTS   "D"


/****** MENSSAGE *******/
#define START_MARKER 0b01111110
#define DATA_SIZE_BYTES 63

#define MSG_MIN_SIZE 12
#define MSG_MAX_SIZE 70

// #define DEBUG_PROTOCOL

#define LO_MODE

typedef struct {
    uint8_t type;         // Tipo = 6bits
    uint8_t size;         // Tamanho da mensagem = 6bits
    uint8_t sequence;     // Sequencia da mensagem = 4bits
    vector<uint8_t> data_bytes; // Dados = 0 ~ 63bytes
    uint8_t parity;   // Paridade 8bits
} msg_t;

typedef struct {
    int sequence;     // Sequencia das mensagens
    int type;         // Se cliente ou servidor
    int socket;
    int target_sequence; // Sequencia do outro

    // Para quando a resposta é perdida e é 
    // necessário reenviar a resposta
    msg_t *last_msg; 
    int last_msg_replied;
    int last_msg_received;
    int can_retry;
} app_info_t;

/****** FUNCTIONS ******/

void init_protocol(int type, int socket, int sequence, int target_sequence);

int send_message(uint8_t type, fstream& data, string param_str="");

int receive_file(uint8_t type, fstream& data);

vector<uint8_t> charToVector(char *data, int size);

string vectorToString(vector<uint8_t> &data, int size);

msg_t *get_message();

void print_error(msg_t *error_msg);

#endif
