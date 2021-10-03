#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>

#define MAXBUFLEN 100

#define PACKET_MAXBUFLEN 1100
#define PACKET_MAXDATALEN 1000

typedef struct packet
{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char * filename;
    char filedata[1000];
} Packet;

void pack_packet(const Packet * packet, char * package)
{
    // Initialize empty buffer
    memset(package, 0, PACKET_MAXBUFLEN);

    // Construct packet members
    sprintf(package, "%d:%d:%d:%s:", packet->total_frag, packet->frag_no, packet->size, packet->filename);
    memcpy(package + strlen(package), packet->filedata, sizeof(char) * PACKET_MAXDATALEN);
}

#endif