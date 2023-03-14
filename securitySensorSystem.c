#include <stdio.h>

#include "WebCam/Recorder/Recorder.h"
#include "MotionSensor/motionSensorController.h"

int main(int argc, char **argv)
{
    printf("hello world\n");
    Recorder_startRecording(300, "test.raw");

    MotionSensorController_start();
    MotionSensorController_stop();

    return 0;
}