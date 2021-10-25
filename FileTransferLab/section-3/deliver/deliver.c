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
#include <sys/time.h>

// Packet
#include "../packet.h"

void send_file_as_packets (char * filename, int sockfd, struct sockaddr_in serv_addr)
{
    struct timeval timeout;
    fd_set readfds;
    int packet_len;
    double sample_RTT = 0, estimated_RTT = 0, dev_RTT = 0, t1 = 99999;
    socklen_t addr_len = sizeof serv_addr;
    struct timeval begin, end;



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
    fseek(pFile, 0, SEEK_SET);

    // Convert file into packets
    Packet packets[total_frag];
    for (int frag_no = 1; frag_no <= total_frag; ++frag_no)
    {
        // Instantiate packet as pointer
        // Initialize all its members
        packets[frag_no - 1].total_frag = total_frag;
        packets[frag_no - 1].frag_no = frag_no;
        packets[frag_no - 1].filename = filename;
        
        // Retrieve data for packet
        packets[frag_no - 1].size = fread(packets[frag_no - 1].filedata, sizeof(char), PACKET_MAXDATALEN, pFile);
    }

    // Close the file
    fclose(pFile);

    // Packet to store for acknowledgement packets
    char ACK_buf[MAXBUFLEN];
    for (int frag_no = 1; frag_no <= total_frag;)
    {
        char * packed_packet = pack_packet(&packets[frag_no - 1], &packet_len);

        // Start the timer
        gettimeofday(&begin, NULL);

        // Send packet through socket
        ///printf("sending packet %d...\n", frag_no);
        if (sendto(sockfd, packed_packet, packet_len, 0, (struct sockaddr *)&serv_addr, addr_len) == -1)
        {
            perror("talker (send_file_as_packets): sendto");
            exit(1);
        }

        // Zero out read file descriptors
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Check for timeouts
        timeout.tv_sec = 0;
        timeout.tv_usec = t1;

        // Set timeout
        if (select(sockfd + 1, &readfds, NULL, NULL, &timeout) == 0)
        {
            // Timeout
            free(packed_packet);
            printf("retransmitting packet %d...\n", frag_no);
            continue;
        }

        // Receive ACK packet
        memset(ACK_buf, 0, MAXBUFLEN);
        if (recvfrom(sockfd, ACK_buf, MAXBUFLEN, 0, (struct sockaddr *)&serv_addr, &addr_len) == -1)
        {
            perror("talker (send_file_as_packets): recvfrom");
            exit(1);
        }

        // End clock
        gettimeofday(&end, NULL);
        
        // Recalculate timeout interval for future packets
        sample_RTT = (end.tv_usec - begin.tv_usec);
        estimated_RTT = (1 - 0.125) * estimated_RTT + (0.125 * sample_RTT);
        dev_RTT = (1 - 0.25) * dev_RTT + (0.25) * fabs(sample_RTT - estimated_RTT);
        t1 = 4 * estimated_RTT;

        // Perform assertions (validations) for ACK packet
        if (strcmp(ACK_buf, "ACK") == 0)
        {
            // Move onto next packet
            ++frag_no;
            free(packed_packet);
        }
    }
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
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
	
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
    gettimeofday(&end, NULL);
    double initial_RTT = (end.tv_usec - begin.tv_usec);
    printf("Total round-trip time: %g microseconds\n", initial_RTT);
	
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
