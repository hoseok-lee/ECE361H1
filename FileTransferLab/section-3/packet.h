#ifndef PACKET_H
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
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char * filename;
    char filedata[PACKET_MAXDATALEN];
} Packet;

char * pack_packet (Packet * packet, int * len)
{
    // Initialize empty buffer
    char * package = malloc(sizeof(char) * PACKET_MAXBUFLEN);
    memset(package, 0, PACKET_MAXBUFLEN);

    // Construct packet members
    sprintf(package, "%d:%d:%d:%s:", packet->total_frag, packet->frag_no, packet->size, packet->filename);
    
    // Determine header size
    int header_size = strlen(package);

    // Use memcpy() instead of string functions to avoid data corruption
    memcpy(package + header_size, packet->filedata, packet->size);
    // Record the total length of the package
    *len = header_size + packet->size;

    return package;
}

Packet * unpack_packet (char * package)
{
    // Collect all packet data through string tokens
    Packet * packet = malloc(sizeof(Packet));
    packet->total_frag = atoi(strtok(package, ":"));
    packet->frag_no = atoi(strtok(NULL, ":"));
    packet->size = atoi(strtok(NULL, ":"));
    packet->filename = strtok(NULL, ":");

    // Determine the header size
    // Add 4 to account for the 4 ":" characters
    int header_size = strlen(strtok(package, ":")) + snprintf(NULL, 0, "%d", packet->frag_no) + snprintf(NULL, 0, "%d", packet->size) + strlen(packet->filename) + 4;
    
    // Use memcpy() instead of string functions to avoid data corruption
    memcpy(packet->filedata, &package[header_size], packet->size);
    // For the last packet, indicate the end of data
    if (packet->size < 1000) {
        packet->filedata[packet->size] = '\0';
    }

    return packet;
}

#endif
