#ifndef MOTION_SENSOR_CONTROLLER_H_
#define MOTION_SENSOR_CONTROLLER_H_

/**
 * Controller for the motion sensor
*/

#include <stdbool.h>

#define TRIGGER_NONE    0x1
#define TRIGGER_CAMERA  0x2


// start the controller thread
void MotionSensorController_start(void);

// returns the number of triggers
int MotionSensorController_getNumTriggers(void);
void MotionSensorController_resetNumTriggers(void);

// toggles the motion sensor on or off
bool MotionSensorController_toggle(void);

// stop the controller thread and cleanup any resources
void MotionSensorController_stop(void);


#endif