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
	
int main(int argc, char **argv)
{
    int sockfd, port;
	struct sockaddr_in serv_addr;



    // deliver <server address> <server port number>
    if (argc != 3)
    {
        exit(0);
    }
	
	port = atoi(argv[2]);
	// Server address information
    memset(&hints, 0, sizeof hints);    // Reset struct to all zeros
    hints.ai_family = AF_INET;          // Use IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP stream sockets
    hints.ai_protocol = IPPROTO_UDP;    // Internet protocol suite for UDP
    hints.ai_flags = AI_PASSIVE;        // Address of local host to sockets

    // Generate socket descriptor for binding
    // sockfd is the socket file descriptor
    if ((sockfd = socket(hints.ai_family, hints.ai_socktype, 0)) == -1)
    {
        perror("talker: socket");
        exit(1);
    }

    // Server socket address information
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	memset((char *) &serv_addr, 0, sizeof serv_addr);
	if (inet_aton(argv[1], &serv_addr.sin_addr) == 0)
    {
		perror("talker: invalid address");
        exit(1);
	}
	
	const int BUF_SIZE = 100;
	char buf[BUF_SIZE] = {0};
	char filename[BUF_SIZE] = {0};
	
	fgets(buf, BUF_SIZE, stdin);
	
	int cursor = 0;
	while(buf[cursor] == ' '){
		cursor++;
	}
	if(tolower(buf[cursor]) == 'f' && tolower(buf[cursor + 1]) == 't' && tolower(buf[cursor + 2]) == 'p'){
		cursor += 3;
	while (buf[cursor] == ' '){ 
		cursor++;
	}
	char *token = strtok(buf + cursor, "\r\t\n ");
	strncpy(filename, token, BUF_SIZE);
	} 
	
	if(access(filename, F_OK) == -1){
		printf("File \"%s\" doesn't exist.\n", filename);
	}
	
	int numbytes;
	// send the message
	if ((numbytes = sendto(sockfd, "ftp", strlen("ftp") , 0 , (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1){
		printf("Sending error\n");
	}
	
	memset(buf, 0, BUF_SIZE); 
	socklen_t serv_addr_size = sizeof(serv_addr);
	if((numbytes = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_addr_size)) == -1){
		printf(stderr, "Receiving error\n");
	}
	
	if(strcmp(buf, "yes") == 0){
		printf("A file transfer can start\n");
	}
	
	close(sockfd);
	
	return 0;
	}
    
