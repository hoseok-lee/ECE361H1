/*#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAXBUFLEN 100

#define PACKET_MAXBUFLEN 1100
#define PACKET_MAXDATALEN 1000

typedef struct packet
{
    unsigned int type;  
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
} Packet;

char * pack_packet (Packet * packet, int * len)
{

}

Packet * unpack_packet (char * package)
{

}

#endif
*/