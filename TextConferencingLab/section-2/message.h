#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define LOGIN 1
#define LO_ACK 2
#define LO_NAK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NAK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13

#define MAX_NAME 50
#define MAX_DATA 500

#define MESSAGE_MAXBUFLEN 600

typedef struct users
{
    int socket;
    char ip_address[INET6_ADDRSTRLEN];
    char name[MAX_NAME];
    char session_ID[MAX_NAME];
    in_port_t port_number;
} Users;

typedef struct message
{
    int type;
    int size;
    char source[MAX_NAME];
    char data[MAX_DATA];
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
