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

void pack_packet (const Packet * packet, char * package)
{
    // Initialize empty buffer
    memset(package, 0, PACKET_MAXBUFLEN);

    // Construct packet members
    sprintf(package, "%d:%d:%d:%s:", packet->total_frag, packet->frag_no, packet->size, packet->filename);
    memcpy(package + strlen(package), packet->filedata, packet->size);
}

void unpack_packet (char * package, Packet * packet)
{
    /*
    // Compile regex to match ":"
    // Use regex to avoid string functions such as strtok()
    // which will cause a segementation fault
    regex_t regex;
    if (regcomp(&regex, "[:]", REG_EXTENDED)) 
    {
        perror("unpack_packet: regcomp");
        exit(1);
    }

    // Match regex to find ":" 
    regmatch_t pmatch[1];
    int ptr = 0;
    char buf[PACKET_MAXBUFLEN];

    // total_frag
    regexec(&regex, package + ptr, 1, pmatch, REG_NOTBOL);
    memset(buf, 0, sizeof(char) * PACKET_MAXBUFLEN);
    memcpy(buf, package + ptr, pmatch[0].rm_so);
    packet->total_frag = atoi(buf);
    ptr += (pmatch[0].rm_so + 1);

    // frag_no
    regexec(&regex, package + ptr, 1, pmatch, REG_NOTBOL);
    memset(buf, 0, sizeof(char) * PACKET_MAXBUFLEN);
    memcpy(buf, package + ptr, pmatch[0].rm_so);
    packet->frag_no = atoi(buf);
    ptr += (pmatch[0].rm_so + 1);

    // size
    regexec(&regex, package + ptr, 1, pmatch, REG_NOTBOL);
    memset(buf, 0, sizeof(char) * PACKET_MAXBUFLEN);
    memcpy(buf, package + ptr, pmatch[0].rm_so);
    packet->size = atoi(buf);
    ptr += (pmatch[0].rm_so + 1);

    // filename
    regexec(&regex, package + ptr, 1, pmatch, REG_NOTBOL);
    memcpy(packet->filename, package + ptr, pmatch[0].rm_so);
    packet->filename[pmatch[0].rm_so] = 0;
    ptr += (pmatch[0].rm_so + 1);
    
    // filedata
    memcpy(packet->filedata, package + ptr, packet->size);
    */
    
    // total_frag
    packet->total_frag = atoi(strtok(package, ":"));
    // frag_no
    packet->frag_no = atoi(strtok(NULL, ":"));
    // size
    packet->size = atoi(strtok(NULL, ":"));
    // filename
    strcpy(packet->filename, strtok(NULL, ":"));
    
    // filedata
    int header_size = strlen(strtok(package, ":")) + snprintf(NULL, 0, "%d", packet->frag_no) + snprintf(NULL, 0, "%d", packet->size) + strlen(packet->filename) + 4;
    memcpy(packet->filedata, &package[header_size], packet->size);
    if (packet->size < PACKET_MAXDATALEN)
    {
        packet->filedata[packet->size] = '\0';
    }
}

#endif
