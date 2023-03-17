#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include "clientNet.h"

#include "../Networking/networking.h"


// ------------------------- PRIVATE ------------------------- //
static int socketDescriptor = 0;
static struct sockaddr_in sinRemote;

// ------------------------- PUBLIC ------------------------- //

void ClientNet_init(void) {
    memset(&sinRemote, 0, sizeof(sinRemote));
    sinRemote.sin_family = AF_INET;
    sinRemote.sin_addr.s_addr = htonl(INADDR_ANY);
    sinRemote.sin_port = htons(SERVER_PORT);

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        perror("socket failed");
        exit(1);
    }
}

// will add a 0 character at the end of message
// the buffer must have a size of RESPONSE_PACKET_SIZE
void ClientNet_receive(char *message) {
    struct sockaddr_in ignore;
    unsigned int sin_len = sizeof(ignore);
    int bytesRx = recvfrom(
        socketDescriptor, 
        message, 
        RESPONSE_PACKET_SIZE - 1, 
        0, 
        (struct sockaddr *)&ignore,
        &sin_len
    );
    message[bytesRx] = 0;
}


/**
 * sends everything in message to host
 * the numBytes <= RELAY_PACKET_SIZE
*/
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


