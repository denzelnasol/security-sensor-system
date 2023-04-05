#ifndef ETHERNET_CLIENT_SYSTEM_H_
#define ETHERNET_CLIENT_SYSTEM_H_

#include <stdbool.h>

void Camera_init(void);

// toggle on or off. true means ok and false means it failed
bool Camera_toggle(void);

// if the camera is on it does nothing otherwise it will turn on the camera
bool Camera_turnOn(void);
// if the camera is off it does nothing otherwise it will turn off the camera
bool Camera_turnOff(void);

// returns true if the camera is on false otherwise
bool Camera_isLive(void);


// this will send the shutdown signal to the camera server
void Camera_cleanup(void);


#endif