#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "motionSensor.h"

#include "../Utilities/utilities.h"
#include "../Timer/timer.h"


#define GPIO_31                             31

#define GPIO_31_BASE                        "/sys/class/gpio/gpio31"

#define GPIO_31_DIRECTION                   GPIO_31_BASE "/direction"
#define GPIO_31_VALUE                       GPIO_31_BASE "/value"

#define ACTIVE_DURATION_MS                  2000


// ------------------------- PRIVATE ------------------------- //

static Timer timer;
static bool inDetectState = true;

static pthread_mutex_t s_fdMutex = PTHREAD_MUTEX_INITIALIZER;
static bool isActive = true;
static pthread_mutex_t s_toggleMutex = PTHREAD_MUTEX_INITIALIZER;

static void setAsInput() 
{
    Utilities_writeStringValueToFile("in", GPIO_31_DIRECTION);
}

static void exportPin()
{
    Utilities_exportGpioPin(GPIO_31_BASE, GPIO_31);
}

static void configPin() 
{
    Utilities_runCommand("config-pin p9.13 gpio");
}

static bool isEnabled()
{
    bool value = false;
    pthread_mutex_lock(&s_toggleMutex);
    {
        value = isActive;
    }
    pthread_mutex_unlock(&s_toggleMutex);
    return value;
}

static PIRState getState()
{
    if (!isEnabled()) {
        return PIR_NONE;
    }

    int value = 0;
    pthread_mutex_lock(&s_fdMutex);
    {
        value = Utilities_readGpioValue(GPIO_31_VALUE);
    }
    pthread_mutex_unlock(&s_fdMutex);
    return value == 1 ? PIR_DETECT : PIR_NONE;
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
    PIRState state = getState();
    if (inDetectState) {
        if (state == PIR_DETECT) {
            Timer_start(ACTIVE_DURATION_MS, &timer);
            inDetectState = false;
            return PIR_DETECT;
        }
    } else {
        if (Timer_isExpired(&timer)) {
            inDetectState = true;
        }
    }
    return PIR_NONE;
}

bool MotionSensor_toggle(void)
{
    bool isTurnOn = false;
    pthread_mutex_lock(&s_toggleMutex);
    {
        isActive = !isActive;
        isTurnOn = isActive;
    }
    pthread_mutex_unlock(&s_toggleMutex);
    return isTurnOn;
}
bool MotionSensor_isEnabled(void)
{
    bool isTurnOn = false;
    pthread_mutex_lock(&s_toggleMutex);
    {
        isTurnOn = isActive;
    }
    pthread_mutex_unlock(&s_toggleMutex);
    return isTurnOn;
}

// cleanup resources
void MotionSensor_cleanup(void)
{
    // nothing to cleanup
}
