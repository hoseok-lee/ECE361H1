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

void send_file_as_packets (char * filename, int sockfd, struct sockaddr_in serv_addr)
{
    socklen_t addr_len = sizeof serv_addr;



    // Open filename file
    FILE * pFile;
    if ((pFile = fopen(filename, "r")) == NULL)
    {
        perror("talker (send_file_as_packets): file open");
        exit(1);
    }

    // Determine number of fragments to be sent
    // Each fragment is 1000 bytes, and if the file is larger than 1000 bytes, 
    // then it must be fragmented
    fseek(pFile, 0L, SEEK_END);
    int file_size = ftell(pFile);
    int total_frag = file_size / PACKET_MAXDATALEN + 1;
    rewind(pFile);

    // Convert file into packets
    // Array of packet information as strings for sending
    char ** packets = malloc(sizeof(char *) * total_frag);

    for (int frag_no = 1; frag_no <= total_frag; ++frag_no)
    {
        // Instantiate packet as pointer
        // Initialize all its members
        Packet * packet = malloc(sizeof(char) * PACKET_MAXBUFLEN);
        packet->total_frag = total_frag;
        packet->frag_no = frag_no;
        packet->filename = filename;
        
        // If the packet is not the last fragment, then the size will be the
        // maximum packet size allowed (1000 bytes)
        // Else if the packet is the last fragment, then the size will be the
        // remaining amount of data in the file
        packet->size = (frag_no < total_frag) ? PACKET_MAXDATALEN : (file_size % PACKET_MAXDATALEN);

        // Retrieve data for packet
        memset(packet->filedata, 0, sizeof(char) * PACKET_MAXDATALEN);
        fread((void *)packet->filedata, sizeof(char), packet->size, pFile);

        // Store in packet array
        packets[frag_no - 1] = malloc(sizeof(char) * PACKET_MAXBUFLEN);
        pack_packet(packet, packets[frag_no - 1]);
    }
    
    // Packet to store for acknowledgement packets
    char ACK_buf[MAXBUFLEN];
    Packet ACK_packet;
    ACK_packet.filename = (char * ) malloc(sizeof(char) * MAXBUFLEN);
    memset(ACK_packet.filename, 0, MAXBUFLEN);

    for (int frag_no = 1; frag_no <= total_frag;  ++frag_no)
    {
        // Send packet through socket
        printf("sending packet %d...\n", frag_no);
        if (sendto(sockfd, packets[frag_no - 1], MAXBUFLEN, 0, (struct sockaddr *)&serv_addr, addr_len) == -1)
        {
            perror("talker (send_file_as_packets): sendto");
            exit(1);
        }

        // Receive ACK packet
        memset(ACK_buf, 0, MAXBUFLEN);
        if (recvfrom(sockfd, ACK_buf, MAXBUFLEN, 0, (struct sockaddr *)&serv_addr, &addr_len) == -1)
        {
            perror("talker (send_file_as_packets): recvfrom");
            exit(1);
        }

        // Unpack the message into a Packet struct
        unpack_packet(ACK_buf, &ACK_packet);

        // Perform assertions (validations) for ACK packet
        if ((strcmp(ACK_packet.filedata, "ACK") == 0) && \
            (ACK_packet.frag_no == frag_no))
        {
            continue;
        }
        else
        {
            perror("received NACK packet...\n");
            exit(1);
        }
    }

    // Deallocate all malloc-ed memory
    fclose(pFile);
    for (int frag_no = 1; frag_no <= total_frag; ++frag_no)
    {
        free(packets[frag_no - 1]);
    }
    free(packets);
    free(ACK_packet.filename);
}
	
int main (int argc, char ** argv)
{
    int sockfd;
    struct addrinfo hints, * servinfo, * p;
	struct sockaddr_in serv_addr;
    socklen_t addr_len;
	char buf[MAXBUFLEN];
	char filename[MAXBUFLEN];



    // Check for number of arguments
    if (argc != 3)
    {
        perror("usage: deliver <server address> <server port number>");
        exit(0);
    }
	
    // Initialize empty buffer
	memset(buf, 0, MAXBUFLEN);
	memset(filename, 0, MAXBUFLEN);

    // Generate socket descriptor for binding
    // sockfd is the socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("talker: socket");
        exit(1);
    }

    // Server socket address information
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], &serv_addr.sin_addr) == 0)
    {
		perror("talker: invalid address");
        exit(1);
	}
	
    // Parse filename
    printf("usage: ftp <file name> ");
	fgets(buf, MAXBUFLEN, stdin);
    
    // Check if the first command is "ftp"
    char * ptr = strtok(buf, " \n");
    if (strcmp(ptr, "ftp") != 0)
    {
        printf("Invalid command.\n");
        exit(1);
    }

    // Check if the next argument exists
    if (ptr == NULL)
    {
        printf("Filename not provided.\n");
        exit(1);
    }

    ptr = strtok(NULL, " \n");
    strncpy(filename, ptr, MAXBUFLEN);

    // Check if file with filename exists
	if(access(filename, F_OK) == -1)
    {
		printf("File \"%s\" doesn't exist.\n", filename);
        exit(1);
	}

    // Measure the round-trip time from the client to the server
    // Start the timer
    clock_t begin, end;
    begin = clock();
	
    // Send message to server
    char * message = "ftp";
	addr_len = sizeof serv_addr;
	if ((sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&serv_addr, addr_len)) == -1)
    {
		perror("talker: sendto");
        exit(1);
	}
	
    // Receive message from client
	if ((recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr *)&serv_addr, &addr_len)) == -1)
    {
		perror("talker: recvfrom");
        exit(1);
	}

    // End the timer
    end = clock();
    printf("Total round-trip time: %g milliseconds\n", (((double) (end - begin)) / CLOCKS_PER_SEC) * 1000);
	
    // File transfer can start once acknowledgement message has been sent
	if (strcmp(buf, "yes") == 0)
    {
		printf("A file transfer can start\n");
	}

    // Send packets based on ACK receipts
    send_file_as_packets(filename, sockfd, serv_addr);
	
	close(sockfd);
    
	return 0;
}
