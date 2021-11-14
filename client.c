/*#include <stdio.h>
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
#include "../packet.h"

void login ()
{
    Send LOGIN packet, receive LO_ACK or LO_NAK
}

void logout ()
{
    send EXIT packet
}

void join_session ()
{
    send JOIN packet, receive JN_ACK or JN_NAK
}

void leave_session ()
{
    send LEAVE_SESS packet
}

void create_session ()
{
    send NEW_SESS packet, receive NS_ACK packet
}

void list ()
{
    send QUERY packet, receive QU_ACK packet
}

void quit ()
{

}

void send_msg ()
{
    send MESSAGE packet
}

int main(int argc, char* argv[]) 
{
    while (1):
        if input is login:
            login()
        if input is logout:
            logout()
        if input is join:
            join_session()
        if input is leave:
            leave_session()
        if input is create:
            create_session()
        if input is list:
            list()
        if input is quit:
            quit()
        else
            send_text()
    return 0;
}*/