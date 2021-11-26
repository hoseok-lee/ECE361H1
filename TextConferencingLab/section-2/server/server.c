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

#define MAX_USERS 4

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
        "guest",
        "jane",
        "john"
    };
    char * password_database[MAX_USERS] = {
        "admin", 
        "guest",
        "abcd",
        "1234"
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
        perror("server: socket");
        exit(1);
    }

    // Skip "address already in use" error message
    int silence_error = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &silence_error, sizeof(int)))
    {
        perror("server: setsockopt");
        exit(1);
    }

    // Bind socket to a port
    printf("binding...\n");
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("server: bind");
        exit(1);
    }

    // Listening...
    printf("listening...\n");
    int incoming_queues = 20;
    if (listen(sockfd, incoming_queues) == -1)
    {
        perror("server: listen");
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
        strcpy(user_list[i].session_ID, "\0");
        user_list[i].port_number = -1;
    }

    while (true)
    {
        // Update the current list of sockets from master
        readfd = master;

        // selecting...
        printf("selecting...\n");
        if (select(maxfd + 1, &readfd, NULL, NULL, NULL) == -1)
        {
            perror("server: select");
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
                if ((newfd = accept(sockfd, (struct sockaddr *)&addr, &addr_len)) == -1)
                {
                    perror("server: accept");
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
                memset(package, 0, MESSAGE_MAXBUFLEN);
                if ((message_len = recv(i, package, MESSAGE_MAXBUFLEN - 1, 0)) == -1)
                {
                    // On error, remove socket descriptor from master list
                    close(i);
                    FD_CLR(i, &master);
                }
                package[MESSAGE_MAXBUFLEN] = '\0';
                
                // Parse package
                int type = atoi(strtok(package, ":"));
                int size = atoi(strtok(NULL, ":"));
                char * username = strtok(NULL, ":");

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

                // User attempts to log in and is currently not already logged in
                if ((type == LOGIN) && (logged == false))
                {
                    // Gather password from message package
                    char * password = strtok(NULL, "\n");

                    printf("%s, %s\n", username, password);

                    // Check if user is already in the list of users
                    bool online = false;
                    for (int j = 0; j < MAX_USERS; ++j)
                    {
                        if (strcmp(user_list[j].name, username) == 0)
                        {
                            online = true;
                            break;
                        }
                    }

                    // Check for credentials in user database
                    bool user_found = false;
                    for (int j = 0; (j < MAX_USERS) && (online == false); ++j)
                    {
                        // Match found
                        if ((strcmp(username_database[j], username) == 0) && \
                            (strcmp(password_database[j], password) == 0))
                        {
                            // Find the first available user spot
                            for (int k = 0; k < MAX_USERS; ++k)
                            {
                                // Socket is not yet set
                                if (user_list[k].socket == -1)
                                {
                                    // Get their socket information
                                    struct sockaddr_in their_addr;
                                    socklen_t addr_len = sizeof their_addr;
                                    getpeername(i, (struct sockaddr *)&their_addr, &addr_len);

                                    char ip_address[INET6_ADDRSTRLEN];
                                    inet_ntop(AF_INET, &(their_addr.sin_addr), ip_address, INET_ADDRSTRLEN);

                                    // Update user information
                                    user_list[k].socket = i;
                                    strcpy(user_list[k].ip_address, ip_address);
                                    strcpy(user_list[k].name, username);
                                    strcpy(user_list[i].session_ID, "\0");
                                    user_list[k].port_number = their_addr.sin_port;
                                    break;
                                }
                            }

                            // Send acknowledgement message
                            serv_message.type = LO_ACK;
                            serv_message.size = 0;

                            // Found a user
                            user_found = true;
                            break;
                        }
                    }

                    // If no user was found, send a NACK packet
                    if (user_found == false)
                    {
                        char * error_message = "wrong credentials";
                        serv_message.type = LO_NAK;
                        serv_message.size = strlen(error_message);
                        strcpy(serv_message.data, error_message);
                    }

                    // Pack and send message
                    char * packed_message = pack_message(&serv_message);
                    if (send(i, packed_message, strlen(packed_message), 0) == -1)
                    {
                        perror("client (LO_ACK): send");
                        exit(1);
                    }
                    free(packed_message);
                }
                else if (type == JOIN)
                {
                    // Gather session from package
                    char * session_ID = strtok(NULL, "\n");

                    // Chcek if session exists
                    bool session_exists = false;
                    for (int j = 0; j < MAX_USERS; ++j)
                    {
                        if (strcmp(user_list[j].session_ID, session_ID) == 0)
                        {
                            session_exists = true;
                            break;
                        }
                    }

                    if (session_exists == true) 
                    {
                        // Send ACK to acknowledge joining session
                        serv_message.type = JN_ACK;
                        serv_message.size = 0;

                         // Add user to the session
                        for (int j = 0; j < MAX_USERS; ++j)
                        {
                            if (user_list[j].socket == i)
                            {
                                strcpy(user_list[j].session_ID, session_ID);
                                break;
                            }
                        }
                    }
                    // Send a NACK if session does not exist
                    else
                    {
                        char error_message[MAX_DATA];
                        sprintf(error_message, "session %s does not exist", session_ID);

                        serv_message.type = JN_NAK;
                        serv_message.size = strlen(error_message);
                        strcpy(serv_message.data, error_message);
                    }

                    // Pack and send message
                    char * packed_message = pack_message(&serv_message);
                    if (send(i, packed_message, strlen(packed_message), 0) == -1)
                    {
                        perror("client (JN_ACK): send");
                        exit(1);
                    }
                    free(packed_message);
                }
                else if (type == LEAVE_SESS)
                {
                    // Remove user from the session
                    for (int j = 0; j < MAX_USERS; ++j)
                    {
                        // Reset to default
                        if (user_list[j].socket == i)
                        {
                            strcpy(user_list[j].session_ID, "\0");
                            break;
                        }
                    }
                }
                else if (type == NEW_SESS)
                {
                    // Gather session from package
                    char * session_ID = strtok(NULL, "\n");

                    // Update user
                    for (int j = 0; j < MAX_USERS; ++j)
                    {
                        if (user_list[j].socket == i)
                        {
                            strcpy(user_list[j].session_ID, session_ID);
                            break;
                        }
                    }

                    // Send acknowledgement
                    serv_message.type = NS_ACK;
                    serv_message.size = 0;

                    // Pack and send message
                    char * packed_message = pack_message(&serv_message);
                    if (send(i, packed_message, strlen(packed_message), 0) == -1)
                    {
                        perror("client (NS_ACK): send");
                        exit(1);
                    }
                    free(packed_message);
                }
                else if (type == QUERY)
                {
                    memset(serv_message.data, 0, MAX_DATA);
                    // Generate a list of all users
                    for (int j = 0; j < MAX_USERS; ++j)
                    {
                        // As long as the user entry is not empty
                        if (strcmp(user_list[j].name, "\0") != 0)
                        {
                            strcat(serv_message.data, user_list[j].name);
                            strcat(serv_message.data, " <");
                            if (strcmp(user_list[j].session_ID, "\0") == 0)
                            {
                                strcat(serv_message.data, "no session");
                            }
                            else
                            {
                                strcat(serv_message.data, user_list[j].session_ID);
                            }
                            strcat(serv_message.data, ">\n");
                        }
                    }

                    printf("%s", serv_message.data);
                    
                    // ACK package
                    serv_message.type = QU_ACK;
                    serv_message.size = strlen(serv_message.data);

                    // Pack and send message
                    char * packed_message = pack_message(&serv_message);
                    if (send(i, packed_message, strlen(packed_message), 0) == -1)
                    {
                        perror("client (QU_ACK): send");
                        exit(1);
                    }
                    free(packed_message);
                }
                else if (type == EXIT)
                {
                    // Terminate the connection
                    close(i);
                    FD_CLR(i, &master);

                    // Remove client from user list
                    for (int j = 0; j < MAX_USERS; ++j) 
                    {
                        // Reset user list entry
                        if (user_list[j].socket == i) {
                            user_list[j].socket = -1;
                            strcpy(user_list[j].ip_address, "\0");
                            strcpy(user_list[j].name, "\0");
                            strcpy(user_list[i].session_ID, "\0");
                            user_list[j].port_number = -1;
                            break;
                        }
                    }
                }
                else if (type == MESSAGE)
                {
                    // Parse client message
                    char * client_message = strtok(NULL, "\0");

                    // Create package
                    serv_message.type = MESSAGE;
                    strcpy(serv_message.data, username);
                    strcat(serv_message.data, ": ");
                    strcat(serv_message.data, client_message);
                    serv_message.size = strlen(serv_message.data);

                    // Check which session the client is a part of
                    int session_sock;
                    for (int j = 0; j < MAX_USERS; ++j)
                    {
                        if (user_list[j].socket == i)
                        {
                            session_sock = j;
                        }
                    }

                    // Skip clients that are not part of any session
                    if (strcmp(user_list[session_sock].session_ID, "\0") == 0)
                    {
                        continue;
                    }

                    // Iterate through all sockets and determine which one shares sessions
                    for (int j = 0; j < MAX_USERS; ++j)
                    {
                        // But don't send to itself
                        if ((strcmp(user_list[j].session_ID, user_list[session_sock].session_ID) == 0) && (j != session_sock))
                        {
                            // Pack and send message
                            char * packed_message = pack_message(&serv_message);
                            if (send(user_list[j].socket, packed_message, strlen(packed_message), 0) == -1)
                            {
                                perror("client (MESSAGE): send");
                                exit(1);
                            }
                            free(packed_message);
                        }
                    }
                }
            }
        }
    }

    freeaddrinfo(res);
    close(sockfd);

    return 0;
}