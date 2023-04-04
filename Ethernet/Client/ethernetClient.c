#include <string.h>

#include "ethernetClient.h"
#include "clientNet.h"
#include "../ethernet.h"

#include "../../WebCam/Stream/Stream.h"

/**
 * 
 * Module to turn camera on/off
 * 
*/

static bool isCameraOn = false;

static void setCamera(bool enabled)
{
    isCameraOn = enabled;
}

static bool isCameraActive()
{
    return isCameraOn;
}

static bool turnCameraOnRequest()
{
    ClientNet_send(ETH_REQ_CAM_ON, sizeof(ETH_REQ_CAM_ON));

    char buffer[ETH_PACKET_SIZE];
    if (!ClientNet_receive(buffer)) {
        return false;
    }

    return strncmp(buffer, ETH_RESPONSE_OK, sizeof(ETH_RESPONSE_OK)) == 0;
}

static bool turnCameraOffRequest()
{
    ClientNet_send(ETH_REQ_CAM_OFF, sizeof(ETH_REQ_CAM_OFF));

    char buffer[ETH_PACKET_SIZE];
    if (!ClientNet_receive(buffer)) {
        return false;
    }

    return strncmp(buffer, ETH_RESPONSE_OK, sizeof(ETH_RESPONSE_OK)) == 0;
}

void Camera_init(void)
{
    ClientNet_init();
}

// toggle on or off
bool Camera_toggle(void)
{
    bool isActive = isCameraActive();
    if (isActive) {
        if (!turnCameraOffRequest()) {
            return false;
        }
    } else {
        if (!turnCameraOnRequest()) {
            return false;
        }
    }
    setCamera(!isActive);
    return true;
}

bool Camera_turnOn(void)
{
    if (isCameraActive()) {
        return true;
    }
    if (turnCameraOnRequest()) {
        setCamera(true);
        return true;
    }
    return false;
}
bool Camera_turnOff(void)
{
    if (!isCameraActive()) {
        return true;
    }
    if (turnCameraOffRequest()) {
        setCamera(false);
        return true;
    }
    return false;
}

// returns true if the camera is on false otherwise
bool Camera_isLive(void)
{
    return isCameraActive();
}

void Camera_cleanup(void)
{
    ClientNet_send(ETH_REQ_STOP, sizeof(ETH_REQ_STOP));
    ClientNet_cleanup();
}

// just for testing
int main() {
    turnCameraOnRequest();
}
