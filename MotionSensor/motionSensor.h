#ifndef MOTION_SENSOR_H_
#define MOTION_SENSOR_H_

/**
 * Controls the motion sensor component of security camera. No software debouncing
 * because the motion sensor will keep the result at 3.3V for about 2 seconds.
 * 
 * Note: Motion detector uses GPIO 31
*/

typedef enum {
    PIR_DETECT,
    PIR_NONE,
} PIRState;


// initializes the module
void MotionSensor_init(void);


// gets the state of the motion sensor
// if something is detected will return PIR_DETECT otherwise PIR_NONE
PIRState MotionSensor_getState(void);


// cleanup resources
void MotionSensor_cleanup(void);


#endif