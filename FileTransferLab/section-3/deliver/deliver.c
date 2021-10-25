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

void send_file_as_packets (char * filename, int sockfd, struct sockaddr_in serv_addr)
{
    struct timeval tv;
    fd_set readfds;
    int rv, packet_len;
    double sample_RTT = 0, estimated_RTT = 0, dev_RTT = 0, t1;
    socklen_t addr_len = sizeof serv_addr;
    clock_t begin, end;



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
        begin = clock();

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
        tv.tv_sec = t1;
        if ((rv = select(sockfd + 1, &readfds, NULL, NULL, &tv)) == 0)
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
        end = clock();

        // Set the sample RTT time for the current packet
        sample_RTT = ((double) (end - begin)) / CLOCKS_PER_SEC;
        // Calculate estimated RTT
        estimated_RTT = (1 - 0.125) * estimated_RTT + (0.125 * sample_RTT);
        // Calculate deviation RTT
        dev_RTT = (1 - 0.25) * dev_RTT + (0.25) * fabs(sample_RTT - estimated_RTT);
        // Calculated timeout interval
        t1 = estimated_RTT + 4 * dev_RTT;

        // Perform assertions (validations) for ACK packet
        if (strcmp(ACK_buf, "ACK") == 0)
        {  
            // Move onto next packet
            ++frag_no;
            free(packed_packet);
            continue;
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
