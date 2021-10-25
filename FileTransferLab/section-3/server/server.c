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
#include <math.h>

// Packet
#include "../packet.h"

int main (int argc, char ** argv)
{
    int sockfd;
    struct addrinfo hints, * servinfo, *  p;
    struct sockaddr_in serv_addr, their_addr;
    socklen_t addr_len;
    char buf[MAXBUFLEN];
    char package[PACKET_MAXBUFLEN];
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
    serv_addr.sin_port = htons(atoi(argv[1]));      // Convert port number to hosthort 
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

    // Binary file to be created from packet
    FILE * pFile = NULL;
    bool * received;
    while (true)
    {
        // Receive packet from talker
        memset(package, 0, PACKET_MAXBUFLEN);
        if (recvfrom(sockfd, package, PACKET_MAXBUFLEN, 0, (struct sockaddr *)&their_addr, &addr_len) == -1)
        {
            // Client didn't sent anything
            message = "NACK";

            // Return the message with sendto
            if ((sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
            {
                perror("listener (NACK packet): sendto");
                exit(1);
            }

            // Skip rest of logic flow
            continue;
        }

        // Unpack packet into received packet
        Packet * packet = unpack_packet(package);
        //printf("receiving packet %d...\n", packet->frag_no);
        if (pFile == NULL)
        {
            received = (bool *) malloc(sizeof(bool) * packet->total_frag);
            memset(received, false, sizeof(bool) * packet->total_frag);
            pFile = fopen(packet->filename, "w");
        }

        // Check for duplicate packets
        if (received[packet->frag_no - 1])
        {
            free(packet);
            
            if (rand() % 100 > 1) 
            {
                // Generate ACK package and return it
                char * acknowledgement_message = "ACK";
                if ((sendto(sockfd, acknowledgement_message, strlen(acknowledgement_message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
                {
                    perror("listener (duplicate ACK packet): sendto");
                    exit(1);
                }
            }
            else 
            {
                printf("Packet dropped! \n");
            }

            // Skip rest of logic flow
            continue;
        }
        else
        {
            // Mark packet as received
            received[packet->frag_no - 1] = true;
        }

        // Generate ACK package and return it
        char * acknowledgement_message = "ACK";
        if ((sendto(sockfd, acknowledgement_message, strlen(acknowledgement_message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
        {
            perror("listener (ACK packet): sendto");
            exit(1);
        }

        // Write the packet filedata into the binary file created
        if (fwrite(packet->filedata, sizeof(char), packet->size, pFile) != packet->size)
        {
            perror("listener (packet): fwrite");
            exit(1);
        }

        // End if all packets have been sent
        if (packet->frag_no == packet->total_frag)
        {
            printf("closing file stream...\n");
            break;
        }

        // Free memory allocated
        free(packet);
    }

    fclose(pFile);

    close(sockfd);

    return 0;
}
