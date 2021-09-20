#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MAXBUFLEN 100

int main (int argc, char **argv)
{
    int sockfd, port;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    int rv, numbytes;
    struct sockaddr_in serv_addr, their_addr;
    socklen_t addr_len;
    char buf[MAXBUFLEN];



    // server <UDP listen port>
    if (argc != 2) {
        exit(0);
    }

    port = atoi(argv[1]);

    // Server socket address information
    serv_addr.sin_family = AF_INET;                 // Use IPv4
    serv_addr.sin_port = htons(port);               // Convert port number to hosthort 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Convert address request to hostlong

    // Server address information
    memset(&hints, 0, sizeof hints);    // Reset struct to all zeros
    hints.ai_family = AF_INET;          // Use IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP stream sockets
    hints.ai_protocol = IPPROTO_UDP;    // Internet protocol suite for UDP
    hints.ai_flags = AI_PASSIVE;        // Address of local host to sockets

    // Generate socket descriptor for binding
    // sockfd is the socket file descriptor
    if ((sockfd = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol)) == -1)
    {
        perror("listener: socket");
        exit(1);
    }

    // Bind socket to a port
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof serv_addr) == -1)
    {
        close(sockfd);
        perror("listener: bind");
        exit(1);
    }

    /*
    // Server information
    memset(&hints, 0, sizeof hints);    // Reset struct to all zeros
    hints.ai_family = AF_UNSPEC;        // Can either use IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;     // UDP stream sockets
    hints.ai_flags = AI_PASSIVE;        // Assign the address of local host to socket structures

    // Retrieve address information
    // servinfo is a linked list of 1 or more struct addrinfos
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop through all the servers and bind to the first available one
    for (p = servinfo; p!= NULL; p = p->ai_next)
    {
        // Generate socket descriptor for binding
        // sockfd is the socket file descriptor
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==  -1)
        {
            perror("listeneder: socket");
            continue;
        }

        // Bind to a port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        // Allow binding of the first available port
        // If all system calls and procedures have succeeded, stop searching
        break;
    }

    // If the linked list was exhausted, then the server failed to bind to a socket
    if (p == NULL)
    {
        fprintf(stderr, "listener: failed to bind\n");
        exit(1);
    }

    // Free the linked list of struct addrinfos
    freeaddrinfo(servinfo);
    */

    // Listening...
    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    // Determine the return message
    char  *message;
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

    close(sockfd);

    return 0;
}
