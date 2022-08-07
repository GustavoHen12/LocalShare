#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <stdlib.h>
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#include"socket_utils.h"

#define SERVER 0
#define CLIENT 1

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
#define SCRN_TYPE    0b111111

/****** ERROR TYPE******/

/****** MENSSAGE *******/
#define START_MARKER 0b01111110
#define DATA_SIZE_BYTES 63

typedef struct {
    uint8_t type;         // Tipo = 6bits
    uint8_t msg_size; // Tamanho da mensagem = 8bits
    uint8_t sequence;     // Sequencia da mensagem = 6bits
    vector<uint8_t> data_bytes; // Dados = 0 ~ 63bytes
    uint8_t parity;   // Paridade 8bits
} msg_t;

typedef struct {
    int sequence;     // Sequencia das mensagens
    int type;         // Se cliente ou servidor
    int socket;
} app_info_t;

app_info_t app_info;

/****** FUNCTIONS ******/

void init_protocol(int type);

int send_message(uint8_t type, ifstream data, vector<uint8_t>& param, int origin);


#endif