#ifndef JOYSTICK_FAKE_H
#define JOYSTICK_FAKE_H

/**
 * This is a fake joystick module for testing on terminal
 */

typedef enum {
    JOYSTICK_NONE,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_CENTER,
} JoystickInput;


void Joystick_init(void);
void Joystick_cleanup(void);

// handles debouncing
JoystickInput Joystick_getPressed(void);

#endif