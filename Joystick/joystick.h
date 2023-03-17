#ifndef JOYSTICK
#define JOYSTICK

/**
 * The joystick module contains logic for configuring the joystick
 * and retrieving important joystick values. It also handles proper debouncing of the joystick
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