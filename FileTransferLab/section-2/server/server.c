#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

// Packet
#include "../packet.h"

int main (int argc, char ** argv)
{
    int sockfd;
    struct addrinfo hints, * servinfo, *  p;
    struct sockaddr_in serv_addr, their_addr;
    socklen_t addr_len;
    char buf[MAXBUFLEN];
    char packet_buf[PACKET_MAXBUFLEN];
    char filename[MAXBUFLEN];



    // Check for number of arguments
    if (argc != 2) 
    {
        perror("usage: server <UDP listen port>");
        exit(0);
    }

    // Initialize empty buffer
	memset(buf, 0, MAXBUFLEN);

    // Server socket address information
    serv_addr.sin_family = AF_INET;                 // Use IPv4
    serv_addr.sin_port = htons(atoi(argv[1]));               // Convert port number to hosthort 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Convert address request to hostlong

    // Generate socket descriptor for binding
    // sockfd is the socket file descriptor
    printf("creating socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("listener: socket");
        exit(1);
    }

    // Bind socket to a port
    printf("binding...\n");
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof serv_addr) == -1)
    {
        perror("listener: bind");
        exit(1);
    }

    // Listening...
    printf("listening...\n");
    addr_len = sizeof their_addr;
    if (recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    // Determine the return message
    char * message;
    if (strcmp(buf, "ftp") == 0)
    {
        message = "yes";
    }
    else
    {
        message = "no";
    }

    // Return the message with sendto
    if ((sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
    {
        perror("listener: sendto");
        exit(1);
    }

    // Send datafile as packet struct
    Packet packet;
	memset(packet.filename, 0, MAXBUFLEN);
    memset(filename, 0, MAXBUFLEN);

    // Binary file to be created from packet
    FILE * pFile;
    char * base_filename;
    sprintf(base_filename, "%.*s", (int)(strrchr(filename, '.') - filename), filename);
    strcat(base_filename, ".bin");

    printf("%s", base_filename);

    if (recvfrom(sockfd, buf, PACKET_MAXBUFLEN, 0, (struct sockaddr *) &their_addr, &addr_len) == -1)
    {
        printf("listener: recvfrom\n");
        exit(1);
    }

    close(sockfd);

    return 0;
}
