#include <stdio.h>

#include "WebCam/Recorder/Recorder.h"
#include "MotionSensor/motionSensorController.h"
#include "PasswordInput/passwordInput.h"
#include "Joystick/joystick.h"

int main(int argc, char **argv)
{
    printf("hello world\n");
    Recorder_startRecording(300, "test.raw");

    MotionSensorController_start();
    Joystick_init();

    PInputSequence seq = PasswordInput_getInputSequence();
    printf("got\n");
    for (int i = 0; i < seq.size; i++) {
        printf("%d: %d\n", i, seq.input[i]);
    }

    Joystick_cleanup();
    MotionSensorController_stop();
    return 0;
}
