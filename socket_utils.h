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

int ConexaoRawSocket(char *device);