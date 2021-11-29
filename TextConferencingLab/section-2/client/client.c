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
#include <pthread.h>

// Message
#include "../message.h"


// Global variables
bool logged = false;
bool session = false;
bool invitation = false;
char invite_session_ID[MAX_NAME];

void * receive_message (void * sockfd_void)
{
    int sockfd = * (int *) sockfd_void;

    while (true)
    {
        // Wait for server reply
        fflush(stdout);
        char serv_reply[MESSAGE_MAXBUFLEN];
        int reply_len;
        if ((reply_len = recv(sockfd, serv_reply, MESSAGE_MAXBUFLEN - 1, 0)) == -1)
        {
            perror("client: recv");
            exit(1);
        }
        serv_reply[reply_len] = '\0';
        fflush(stdout);

        // Unpack data
        int type = atoi(strtok(serv_reply, ":"));
        int size = atoi(strtok(NULL, ":"));
        char * source = strtok(NULL, ":");
        char * data = strtok(NULL, "\0");

        if (type == QU_ACK)
        {
            // All online users
            printf("%s", data);
        }
        else if (type == NS_ACK)
        {
            printf("Successfully created session!\n");
            session = true;
        }
        else if (type == JN_ACK)
        {
            printf("Successfully joined session!\n");
            session = true;
        }
        else if (type == JN_NAK)
        {
            printf("Session join failure: %s\n", data);
            session = false;
        }
        else if (type == MESSAGE)
        {
            printf("%s", data);
        }
        else if (type == WHISPER)
        {
            printf("%s", data);
        }
        else if (type == WS_NAK)
        {
            printf("Whisper message error: %s\n", data);
        }
        else if (type == INVITE)
        {
            printf("Do you want to join session \"%s\"? ", data);

            // Copy data for prompt
            invitation = true;
            strcpy(invite_session_ID, data);
        }
        else if (type == IN_NAK)
        {
            printf("Invite error: %s\n", data);
        }
    }

    return NULL;
}

int main (int argc, char ** argv)
{
    int sockfd;
    struct addrinfo hints, * res;
    Message message;
    pthread_t receive_proc;



    while (true)
    {
        // Receive user input
        char input[MESSAGE_MAXBUFLEN];
        fgets(input, MESSAGE_MAXBUFLEN, stdin);

        // Check for invitation
        if ((logged == true) && (invitation == true)) {
            if (strcmp(input, "yes\n") == 0)
            {
                // Create new message struct
                message.type = JOIN;
                message.size = strlen(invite_session_ID);
                strcpy(message.data, invite_session_ID);

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }
                free(packed_message);
            }

            // Cancel invitation whether or not receiver said "yes" or anything else
            invitation = false;
        }

        // If the user input begins with a slash to indicate a command
        if (input[0] == '/')
        {
            // Separate the command from the arguments
            char * command = strtok(input, " ");

            // LOGIN
            if (strcmp(command, "/login") == 0)
            {
                // Retrieve arguments
                char * username = strtok(NULL, " ");
                char * password = strtok(NULL, " ");
                char * ip_address = strtok(NULL, " ");
                char * port_number = strtok(NULL, "\n");

                // Create new message struct
                message.type = LOGIN;
                message.size = strlen(password);
                strcpy(message.source, username);
                strcpy(message.data, password);

                // Server socket address information
                memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;

                // Get address information
                getaddrinfo(ip_address, port_number, &hints, &res);

                // Generate socket descriptor for binding
                // sockfd is the socket file descriptor
                if ((sockfd = socket(res->ai_family, res->ai_socktype, 0)) == -1)
                {
                    perror("client: socket");
                    exit(1);
                }

                // Connect to server through socket
                if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
                {
                    perror("client: connect");
                    exit(1);
                }

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }

                // Wait for LO_ACK acknowledgement
                char ack_package[MESSAGE_MAXBUFLEN];
                int ack_len;
                if ((ack_len = recv(sockfd, ack_package, MESSAGE_MAXBUFLEN - 1, 0)) == -1)
                {
                    perror("client: recv");
                    exit(1);
                }
                ack_package[ack_len] = '\0';

                // Unpack ack packet
                int ack_type = atoi(strtok(ack_package, ":"));
                int ack_size = atoi(strtok(NULL, ":"));
                char * source = strtok(NULL, ":");

                // Server acknowledges login attempt
                if (ack_type == LO_ACK)
                {
                    printf("Logged in successfully!\n");
                    logged = true;

                    // Begin receive message thread
                    pthread_create(&receive_proc, NULL, receive_message, (void *)&sockfd);
                }
                // ack_type == LO_NAK
                else
                {
                    char * error_message = strtok(NULL, "\0");
                    printf("Log-in failure: %s\n", error_message);
                }

                // Clean up
                freeaddrinfo(res);
                free(packed_message);
            }
            // JOINSESSION
            else if ((strcmp(command, "/joinsession") == 0) && (logged == true) && (session == false))
            {
                // Retrieve arguments
                char * session_ID = strtok(NULL, "\n");

                // Build join session request message
                message.type = JOIN;
                message.size = strlen(session_ID);
                strcpy(message.data, session_ID);

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }
                free(packed_message);
            }
            // LEAVESESSION
            else if ((strcmp(input, "/leavesession\n") == 0) && (logged == true) && (session == true))
            {
                // Build leave session request message
                message.type = LEAVE_SESS;
                message.size = 0;

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }
                free(packed_message);

                session = false;

                printf("Left session!\n");
            }
            // CREATESESSION
            else if ((strcmp(command, "/createsession") == 0) && (logged == true) && (session == false))
            {
                // Retrieve arguments
                char * session_ID = strtok(NULL, "\n");

                // Build create session request message
                message.type = NEW_SESS;
                message.size = strlen(session_ID);
                strcpy(message.data, session_ID);

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }
                free(packed_message);
            }
            // LIST
            else if ((strcmp(input, "/list\n") == 0) && (logged == true))
            {
                // Build list request message
                message.type = QUERY;
                message.size = 0;

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }
                free(packed_message);
            }
            // QUIT
            else if (strcmp(input, "/quit\n") == 0)
            {
                if (logged == true)
                {
                    // Build exit request message
                    message.type = EXIT;
                    message.size = 0;

                    // Send serialized message
                    char * packed_message = pack_message(&message);
                    if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                    {
                        perror("client: send");
                        exit(1);
                    }
                    free(packed_message);

                    logged = false;
                }

                printf("Exiting...\n");

                return 0;
            }
            // WHISPER
            else if ((strcmp(input, "/whisper") == 0) && (logged == true))
            {
                // Retrieve arguments
                char * receiver = strtok(NULL, " ");
                char * data = strtok(NULL, "\n");

                // Create new message struct
                message.type = WHISPER;
                sprintf(message.data, "%s:%s", receiver, data);
                message.size = strlen(message.data);

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }
                free(packed_message);
            }
            // INVITE
            else if ((strcmp(input, "/invite") == 0) && (logged == true))
            {
                // Retrieve arguments
                char * receiver = strtok(NULL, "\n");

                // Create new message struct
                message.type = INVITE;
                message.size = strlen(receiver);
                strcpy(message.data, receiver);

                // Send serialized message
                char * packed_message = pack_message(&message);
                if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
                {
                    perror("client: send");
                    exit(1);
                }
                free(packed_message);
            }
        }
        // MESSAGE
        else if ((logged == true) && (session == true))
        {
            message.type = MESSAGE;
            message.size = strlen(input);
            strcpy(message.data, input);

            // Send serialized message
            char * packed_message = pack_message(&message);
            if (send(sockfd, packed_message, strlen(packed_message), 0) == -1)
            {
                perror("client: send");
                exit(1);
            }
            free(packed_message);
        }
    }
}
