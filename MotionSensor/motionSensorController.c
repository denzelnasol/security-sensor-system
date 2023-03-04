#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "motionSensor.h"
#include "motionSensorController.h"

#include "../Timer/timer.h"
#include "../Utilities/utilities.h"

#define ACTIVE_DURATION_MS  2000
#define SLEEP_FREQUENCY_MS  10

// ------------------------- PRIVATE ------------------------- //

void logWarning(void);
static void (*onTrigger)(void) = &logWarning;

void* PIRListenerThread(void* arg);
static pthread_t s_PIRThreadId;
static bool s_stoppingSignal = false;
static pthread_mutex_t s_stoppingMutex = PTHREAD_MUTEX_INITIALIZER;

void logWarning(void)
{
    printf("MOTION SENSOR: Intruder detected.\n");
}

static bool isStoppingSignalReceived()
{
    bool received = false;
    pthread_mutex_lock(&s_stoppingMutex);
    {
        received = s_stoppingSignal;
    }
    pthread_mutex_unlock(&s_stoppingMutex);
    return received;
}
static void sendStoppingSignal()
{
    pthread_mutex_lock(&s_stoppingMutex);
    {
        s_stoppingSignal = true;
    }
    pthread_mutex_unlock(&s_stoppingMutex);
}


void *PIRListenerThread(void *args)
{
    Timer timer;
    bool inDetectState = true;
    while (!isStoppingSignalReceived()) {
        PIRState state = MotionSensor_getState();

        if (inDetectState) {
            if (state == PIR_DETECT) {
                (*onTrigger)();
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
    pthread_create(&s_PIRThreadId, &attr, PIRListenerThread, NULL);
}


// sets the trigger options for the motion detector
void MotionSensorController_setTriggerAction(void (*callBack)(void))
{
    onTrigger = callBack;
}


// stop the controller thread and cleanup any resources
void MotionSensorController_stop(void)
{
    sendStoppingSignal();
    pthread_join(s_PIRThreadId, NULL);
}