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
#include <ctype.h>
#include <time.h>

#define MAXBUFLEN 100
	
int main (int argc, char **argv)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in serv_addr;
    socklen_t addr_len;
	char buf[MAXBUFLEN] = { '\0' };
	char filename[MAXBUFLEN] = { '\0' };



    // Check for number of arguments
    if (argc != 3)
    {
        perror("usage: deliver <server address> <server port number>");
        exit(0);
    }
	
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
    char *ptr = strtok(buf, " \n");
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
    char *message = "ftp";
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
	
	close(sockfd);
    
	return 0;
}
