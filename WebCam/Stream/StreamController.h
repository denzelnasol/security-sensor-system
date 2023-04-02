#ifndef STREAM_CONTROLLER_H_
#define STREAM_CONTROLLER_H_

#include "Stream.h"

/**
 * Controller for web cam streaming
*/

// start the controller thread
void Stream_Controller_start(void);


// sets the trigger options for streaming
// void Stream_Controller_setTriggerAction(void (*callBack)(void));


// toggles the stream controller to be on trigger or manual. returns true if its triggered false otherwise
bool Stream_Controller_toggle(void);

// returns true if the camera is currently set to triggered otherwise false
bool Stream_Controller_isTriggered(void);

// stop the controller thread and cleanup any resources
void Stream_Controller_stop(void);

#endif
