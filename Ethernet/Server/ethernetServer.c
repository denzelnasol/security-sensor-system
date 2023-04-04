#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Listen for requests from the client
*/

#include "serverNet.h"
#include "../ethernet.h"

#include "../../WebCam/Stream/Stream.h"


typedef enum {
    SIGNAL_NONE,
    SIGNAL_STOP,
} Signal;


static Signal execute(const char *command, char *response)
{
    bool res;
    if (strncmp(command, ETH_REQ_STOP, sizeof(ETH_REQ_STOP)) == 0) {
        return SIGNAL_STOP;
    }

    if (strncmp(command, ETH_REQ_CAM_OFF, sizeof(ETH_REQ_CAM_OFF)) == 0) {
        res = Stream_turnOff();

    } else if (strncmp(command, ETH_REQ_CAM_ON, sizeof(ETH_REQ_CAM_ON)) == 0) {
        res = Stream_turnOn();
    }

    if (res) {
        snprintf(response, ETH_PACKET_SIZE, ETH_RESPONSE_OK);
    } else {
        snprintf(response, ETH_PACKET_SIZE, ETH_RESPONSE_BAD);
    }

    return SIGNAL_NONE;
}

// ------------------------- PUBLIC ------------------------- //

int main(void)
{
    ServerNet_init();

    char command[ETH_PACKET_SIZE];
    char response[ETH_PACKET_SIZE];

    Signal signal = SIGNAL_NONE;
    while (signal != SIGNAL_STOP) {
        ServerNet_receive(command);

        printf("%s\n", command);

        signal = execute(command, response);
        ServerNet_send(response, strlen(response));
    }

    ServerNet_cleanup();
    return 0;
}
