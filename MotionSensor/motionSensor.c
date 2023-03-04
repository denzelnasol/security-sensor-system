#include <stdio.h>
#include <pthread.h>

#include "motionSensor.h"

#include "../Utilities/utilities.h"


#define EXPORT_PATH                         "/sys/class/gpio/export"

#define GPIO_31                             31

#define GPIO_31_BASE                        "/sys/class/gpio/gpio31"

#define GPIO_31_DIRECTION                   GPIO_31_BASE "/direction"
#define GPIO_31_VALUE                       GPIO_31_BASE "/value"

#define GPIO_SETUP_DELAY_MS                 300

// ------------------------- PRIVATE ------------------------- //

static pthread_mutex_t s_fdMutex = PTHREAD_MUTEX_INITIALIZER;


static void setAsInput() 
{
    Utilities_writeStringValueToFile("in", GPIO_31_DIRECTION);
}

static void exportPin()
{
    if (!Utililties_isFileExists(GPIO_31_BASE)) {
        Utilities_writeIntValueToFile(GPIO_31, EXPORT_PATH);
        Utilities_sleepForMs(GPIO_SETUP_DELAY_MS);
    }
}

static void configPin() 
{
    Utilities_runCommand("config-pin p9.13 gpio");
}

// ------------------------- PUBLIC ------------------------- //

// initializes the module
void MotionSensor_init(void)
{
    configPin();
    exportPin();
    setAsInput();
}


// gets the state of the motion sensor
// if something is detected will return PIR_DETECT otherwise PIR_NONE
PIRState MotionSensor_getState(void)
{
    int value = 0;
    pthread_mutex_lock(&s_fdMutex);
    {
        value = Utilities_readGpioValue(GPIO_31_VALUE);
    }
    pthread_mutex_unlock(&s_fdMutex);
    return value == 1 ? PIR_DETECT : PIR_NONE;
}


// cleanup resources
void MotionSensor_cleanup(void)
{
    // nothing to cleanup
}