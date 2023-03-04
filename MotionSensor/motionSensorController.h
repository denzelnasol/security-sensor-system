#ifndef MOTION_SENSOR_CONTROLLER_H_
#define MOTION_SENSOR_CONTROLLER_H_

/**
 * Controller for the motion sensor
*/

#define TRIGGER_NONE    0x1
#define TRIGGER_CAMERA  0x2


// start the controller thread
void MotionSensorController_start(void);


// sets the trigger options for the motion detector
void MotionSensorController_setTriggerAction(void (*callBack)(void));


// stop the controller thread and cleanup any resources
void MotionSensorController_stop(void);


#endif