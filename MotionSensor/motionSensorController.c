#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "motionSensor.h"
#include "motionSensorController.h"

#include "../EventLogger/logger.h"
#include "../Timer/timer.h"
#include "../Utilities/utilities.h"

// the motion sensor we are using maintains active high for 2s
#define ACTIVE_DURATION_MS  2000
#define SLEEP_FREQUENCY_MS  10

// ------------------------- PRIVATE ------------------------- //

void* mainloop(void* arg);
static pthread_t s_thread;
static bool s_stop = false;
static pthread_mutex_t s_stopMutex = PTHREAD_MUTEX_INITIALIZER;

static bool toggle = true;
static pthread_mutex_t s_toggleMutex = PTHREAD_MUTEX_INITIALIZER;

static int numTriggers = 0;
static pthread_mutex_t s_numTriggersMutex = PTHREAD_MUTEX_INITIALIZER;


static bool isStoppingSignalReceived()
{
    bool received = false;
    pthread_mutex_lock(&s_stopMutex);
    {
        received = s_stop;
    }
    pthread_mutex_unlock(&s_stopMutex);
    return received;
}
static void sendStoppingSignal()
{
    pthread_mutex_lock(&s_stopMutex);
    {
        s_stop = true;
    }
    pthread_mutex_unlock(&s_stopMutex);
}
static void onDetect()
{
    Logger_logWarning("detected movement");
    pthread_mutex_lock(&s_numTriggersMutex);
    {
        numTriggers++;
    }
    pthread_mutex_unlock(&s_numTriggersMutex);
}

void *mainloop(void *args)
{
    Timer timer;
    bool inDetectState = true;
    while (!isStoppingSignalReceived()) {
        PIRState state = MotionSensor_getState();

        if (inDetectState) {
            if (state == PIR_DETECT) {
                onDetect();
                Timer_start(ACTIVE_DURATION_MS, &timer);
                inDetectState = false;
            }
        } else {
            if (Timer_isExpired(&timer)) {
                inDetectState = true;
            }
        }

        Utilities_sleepForMs(SLEEP_FREQUENCY_MS);
    }
    return NULL;
}


// ------------------------- PRIVATE ------------------------- //

// start the controller thread
void MotionSensorController_start(void)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&s_thread, &attr, mainloop, NULL);
}


int MotionSensorController_getNumTriggers(void)
{
    int triggerCount = 0;
    pthread_mutex_lock(&s_numTriggersMutex);
    {
        triggerCount = numTriggers;
    }
    pthread_mutex_unlock(&s_numTriggersMutex);
    return triggerCount;
}
void MotionSensorController_resetNumTriggers(void)
{
    pthread_mutex_lock(&s_numTriggersMutex);
    {
        numTriggers = 0;
    }
    pthread_mutex_unlock(&s_numTriggersMutex);
}
bool MotionSensorController_toggle(void)
{
    bool isTurnOn = false;
    pthread_mutex_lock(&s_toggleMutex);
    {
        toggle = !toggle;
        isTurnOn = toggle;
    }
    pthread_mutex_unlock(&s_toggleMutex);
    return isTurnOn;
}

// stop the controller thread and cleanup any resources
void MotionSensorController_stop(void)
{
    sendStoppingSignal();
    pthread_join(s_thread, NULL);
}