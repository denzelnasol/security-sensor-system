#ifndef PASSWORD_INPUT_H_
#define PASSWORD_INPUT_H_

#include <stdlib.h>

typedef struct {
    const JoystickInput *input;
    size_t size;
} PInputSequence;

// blocks until the joystick is pressed IN
PInputSequence PasswordInput_getInputSequence(void);


#endif