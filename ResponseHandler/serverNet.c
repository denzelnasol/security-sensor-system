#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include "serverNet.h"

#include "../Networking/networking.h"

// ------------------------- PRIVATE ------------------------- //
static int socketDescriptor = 0;
static struct sockaddr_in sinRemote;

// ------------------------- PUBLIC ------------------------- //

void ServerNet_init(void) {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(SERVER_PORT);

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        perror("socket failed");
        exit(1);
    }

    if (bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
        close(socketDescriptor);
        perror("bind failed");
        exit(1);
    }
}

// will add a 0 character at the end of message
void ServerNet_receive(char *message) {
    unsigned int sin_len = sizeof(sinRemote);
    int bytesRx = recvfrom(
        socketDescriptor, 
        message, 
        RELAY_PACKET_SIZE - 1, 
        0, 
        (struct sockaddr *)&sinRemote,
        &sin_len
    );
    message[bytesRx] = 0;
}


// sends everything in message to host
void ServerNet_send(const char *message, int numBytes) {
    sendto(
        socketDescriptor, 
        message, 
        numBytes, 
        0, 
        (struct sockaddr *)&sinRemote,
        sizeof(sinRemote)
    );
}

void ServerNet_cleanup(void) {
    close(socketDescriptor);
}
