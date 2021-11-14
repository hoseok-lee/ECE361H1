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

int main (int argc, char* argv[]) 
{
    setsockopt()
    bind()
    listen()

    while(1):
        if packet_recv is LOGIN:
            login()
        if packet_recv is EXIT:
            logout()
        if packet_recv is JOIN_SESS:
            join_session()
        if packet_recv is LEAVE_SESS:
            leave_session()
        if packt_recv is NEW_SESS:
            create_session()
        if packet_recv is QUERY:
            list()
        if packet_recv is quit:
            quit()
        else
            receive_text() and cast_text()
}
*/