#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include "clientNet.h"
#include "../ethernet.h"

#define TIMEOUT_S   1

// ------------------------- PRIVATE ------------------------- //
static int socketDescriptor = 0;
static struct sockaddr_in sinRemote;

// ------------------------- PUBLIC ------------------------- //

void ClientNet_init(void) {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, ETH_HOST_ADDR, &(sin.sin_addr));
    sin.sin_port = htons(ETH_HOST_PORT);

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        perror("socket failed");
        exit(1);
    }

    // https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    struct timeval tv;
    tv.tv_sec = TIMEOUT_S;
    tv.tv_usec = 0;
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        close(socketDescriptor);
        exit(1);
    }
}

// will add a 0 character at the end of message
bool ClientNet_receive(char *message) {
    struct sockaddr_in ignore;
    unsigned int sin_len = sizeof(ignore);
    int bytesRx = recvfrom(
        socketDescriptor, 
        message, 
        ETH_PACKET_SIZE - 1, 
        0, 
        (struct sockaddr *)&ignore,
        &sin_len
    );
    if (bytesRx < 0) {
        return false;
    }
    message[bytesRx] = 0;
    return true;
}


// sends everything in message to host
void ClientNet_send(const char *message, int numBytes) {
    sendto(
        socketDescriptor, 
        message, 
        numBytes, 
        0, 
        (struct sockaddr *)&sinRemote,
        sizeof(sinRemote)
    );
}

void ClientNet_cleanup(void) {
    close(socketDescriptor);
}


// int main(void) {
//     ClientNet_init();

//     char *str = "hello from client";
//     ClientNet_send(str, sizeof(str));


//     ClientNet_cleanup();
// }
