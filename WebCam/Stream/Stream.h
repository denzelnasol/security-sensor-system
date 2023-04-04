#ifndef BBG_CAM_STREAMING_H_
#define BBG_CAM_STREAMING_H_

typedef struct {
    bool isActive;
    bool isOperationSucceeded;
} StreamingToggle;


void Stream_init(void);
void Stream_cleanup(void);

// toggle on or off
StreamingToggle Stream_toggle(void);

// returns true if the camera is on false otherwise
bool Stream_isLive(void);

#endif
