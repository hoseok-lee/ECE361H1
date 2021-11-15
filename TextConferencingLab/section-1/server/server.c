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
#include <fcntl.h>

// Message
#include "../message.h"

#define MAX_USERS 2

int main (int argc, char ** argv)
{
    int sockfd;
    struct addrinfo hints, * res;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    char package[MESSAGE_MAXBUFLEN];
    Users user_list[MAX_USERS];

    // Userenames and passwords
    char * username_database[MAX_USERS] = {
        "admin", 
        "guest"
    };
    char * password_database[MAX_USERS] = {
        "admin", 
        "guest"
    };



    // Check for number of arguments
    if (argc != 2)
    {
        perror("usage: server <TCP listen port>");
        exit(0);
    }

    // Server socket address information
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get address information
    getaddrinfo(NULL, argv[1], &hints, &res);

    // Generate socket descriptor for binding
    // sockfd is the socket file descriptor
    printf("creating socket...\n");
    if ((sockfd = socket(res->ai_family, res->ai_socktype, 0)) == -1)
    {
        perror("listener: socket");
        exit(1);
    }

    // Skip "address already in use" error message
    int silence_error = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &silence_error, sizeof(int)))
    {
        perror("listener: setsockopt");
        exit(1);
    }

    // Bind socket to a port
    printf("binding...\n");
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("listener: bind");
        exit(1);
    }

    // Listening...
    printf("listening...\n");
    int incoming_queues = 20;
    if (listen(sockfd, incoming_queues) == -1)
    {
        perror("listener: listen");
        exit(1);
    }

    // Create socket descriptor sets
    fd_set master, readfd;
    FD_ZERO(&master);
    FD_ZERO(&readfd);
    FD_SET(sockfd, &master);

    // Keep track of largest socket descriptor for select function
    int maxfd = sockfd;
    addr_len = sizeof(addr);

    // Initialize user list with empty buffers
    for (int i = 0; i < MAX_USERS; ++i)
    {
        user_list[i].socket = -1;
        strcpy(user_list[i].ip_address, "\0");
        strcpy(user_list[i].name, "\0");
        user_list[i].session = -1;
        user_list[i].port = -1;
    }

    while (true)
    {
        // Update the current list of sockets from master
        readfd = master;

        // selecting...
        printf("selecting...\n");
        if (select(maxfd + 1, &readfd, NULL, NULL, NULL) == -1)
        {
            perror("listener: select");
            exit(1);
        }

        // Sever to client message
        struct message serv_message;
        strcpy(serv_message.source, "server");

        // Iterate through all sockets
        for (int i = 0; i < maxfd + 1; ++i)
        {
            // If there isn't anything queued in the socket, skip it
            if (!FD_ISSET(i, &readfd))
            {
                continue;
            }

            // Check the server's socket descriptor for incoming messages
            // Request for new incoming connections
            if (i == sockfd)
            {
                int newfd;
                if ((newfd = accept(sockfd, (struct sockaddr *)&addr, &addr_len))) == -1)
                {
                    perror("listener: accept");
                    exit(1);
                }

                // Add new socket to master list and update max socket descriptor
                FD_SET(newfd, &master);
                if (newfd > maxfd)
                {
                    maxfd = newfd;
                }
            }
            // New data from client has arrived
            else
            {
                // Receive packaged message data from client
                int message_len;
                if ((message_len = recv(i, package, MESSAGE_MAXBUFLEN - 1, 0) <= 0)
                {
                    // On error, remove socket descriptor from master list
                    close(i);
                    FD_CLR(i, &master);
                }
                package[MESSAGE_MAXBUFLEN] = '\0';
                
                // Parse package
                int type = atoi(strtok(package, ":"));
                int size = atoi(strtok(NULL, ":"));
                char * name = strtok(NULL, ":");

                // Check whether the user has already logged in or not
                bool logged = false;
                for (int j = 0; j < MAX_USERS; ++j)
                {
                    if (user_list[j].socket == i)
                    {
                        logged = true;
                        break;
                    }
                }

                // As long as the user is not attempting to log in
                // or is already logged in
                if (!(strcmp(package, "LOGIN")) && !logged)
                {

                }
                else if (!(strcmp(package, "EXIT")))
                {

                }
                else if (!(strcmp(package, "JOIN")))
                {

                }
                else if (!(strcmp(package, "LEAVE_SESS")))
                {

                }
                else if (!(strcmp(package, "NEW_SESS")))
                {

                }
                else if (!(strcmp(package, "NS_ACK")))
                {

                }
                else if (!(strcmp(package, "MESSAGE")))
                {

                }
                else if (!(strcmp(package, "QUERY")))
                {

                }
                else if (!(strcmp(package, "INVITE")))
                {

                }
            }
        }
    }

    freeaddrinfo(res);
    close(sockfd);

    return 0;
}