#ifndef BBG_CAM_STREAMING_H_
#define BBG_CAM_STREAMING_H_

void Stream_init(void);
void Stream_cleanup(void);

// toggle on or off
bool Stream_toggle(void);

// precondition: if the stream is not on
bool Stream_turnOn(void);

// precondition: if the stream is not off
bool Stream_turnOff(void);

#endif
