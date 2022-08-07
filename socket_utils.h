#ifndef __SOCKET_UTILS__
#define __SOCKET_UTILS__

/*
    Abstrai funcoes do RawSocket
        - Cria socket
        - Configura socket
        - Envia
        - Recebe
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>

/*
* Função cria conexão com raw socket e retorna 
* o numero do raw socket
*/
int conect_raw_socket(char *device);

#endif