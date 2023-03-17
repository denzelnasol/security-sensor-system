#ifndef PASSWORD_INPUT_H_
#define PASSWORD_INPUT_H_

#include <stdlib.h>

// #include "../Joystick/joystick.h"
#include "../Menu/MockObjects/joystick.h"

typedef enum {
    P_INPUT_CONTINUE,
    P_INPUT_MATCH,
    P_INPUT_NO_MATCH,
    P_INPUT_TOO_LONG,
} PInputState;


void PasswordInput_init(void);

// something needs to relay the joystick input into this function
// any invalid inputs such as joystick_none will be ignored
// postcondition: if it returns pInputState != CONTINUE the buffer will be reset
PInputState PasswordInput_sendNext(JoystickInput input);

void PasswordInput_cleanup(void);

#endif
