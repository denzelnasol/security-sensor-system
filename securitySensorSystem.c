#include <stdio.h>

#include "MotionSensor/motionSensorController.h"

int main(int argc, char **argv)
{
    printf("hello world\n");

    MotionSensorController_start();
    MotionSensorController_stop();

    return 0;
}