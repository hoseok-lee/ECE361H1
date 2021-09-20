#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>

	
int main(int argc, char const *argv[]){
	
	int port = atoi(argv[2]);
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1){
		printf("Socket error\n");
	}
	
	struct sockaddr_in serv_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	if(inet_aton(argv[1], &serv_addr.sin_addr) == 0){
		printf("Invalid address\n");
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	
	const int BUF_SIZE = 100;
	char buf[BUF_SIZE] = {0};
	char filename[BUF_SIZE] = {0};
	
    printf("Enter a filename in the format: ftp <file name>");
	fgets(buf, BUF_SIZE, stdin);
	
	int i = 0;
	while(buf[i] == ' '){
		i++;
	}
	if(buf[i] == 'f' && buf[i + 1] == 't' && buf[i + 2] == 'p'){
		i += 3;
	while (buf[i] == ' '){ 
		i++;
	}
	char *token = strtok(buf + cursor, "\r\t\n ");
	strncpy(filename, token, BUF_SIZE);
	} 
	
	if(access(filename, F_OK) == -1){
		printf("File \"%s\" doesn't exist.\n", filename);
	}
	
	int bytes = sendto(sockfd, "ftp", strlen("ftp"), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (bytes == -1){
		printf("Sending error\n");
	}
	
	memset(buf, 0, BUF_SIZE); 
	socklen_t serv_addr_size = sizeof(serv_addr);
    bytes = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_addr_size))
	if(bytes == -1){
		printf("Receiving error\n");
	}
	
	if(strcmp(buf, "yes") == 0){
		printf("A file transfer can start\n");
	}
	
	close(sockfd);
	
	return 0;
	}
    
