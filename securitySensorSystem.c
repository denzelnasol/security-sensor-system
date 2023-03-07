#include <stdio.h>

#include "MotionSensor/motionSensorController.h"
#include "PasswordInput/passwordInput.h"
#include "Joystick/joystick.h"

int main(int argc, char **argv)
{
    Joystick_init();

    PInputSequence seq = PasswordInput_getInputSequence();
    printf("got\n");
    for (int i = 0; i < seq.size; i++) {
        printf("%d: %d\n", i, seq.input[i]);
    }

    Joystick_cleanup();
    return 0;
}
