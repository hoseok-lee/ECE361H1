#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_NAME 50
#define MAX_DATA 500

#define MESSAGE_MAXBUFLEN 600

typedef struct users
{
    int socket;
    char ip_address[INET6_ADDRSTRLEN];
    char name[MAX_NAME];
    int session;
    in_port_t port;
} Users;

typedef struct message
{
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
} Message;

char * pack_message (Message * message)
{
    // Initialize empty buffer
    char * package = malloc(sizeof(char) * MESSAGE_MAXBUFLEN);
    memset(package, 0, MESSAGE_MAXBUFLEN);

    // Construct message members
    sprintf(package, "%d:%d:%s:%s", message->type, message->size, message->source, message->data);

    return package;
}

#endif
