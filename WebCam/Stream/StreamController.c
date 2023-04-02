#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "StreamController.h"
#include "Stream.h"

#include "../../Utilities/utilities.h"
#include "../../MotionSensor/motionSensor.h"

#define STREAM_DURATION_MS  120000
#define SLEEP_FREQUENCY_MS  10

// ------------------------- PRIVATE ------------------------- //

void* streamListenerThread(void* arg);
static pthread_t s_streamThreadId;
static pthread_mutex_t s_stoppingMutex = PTHREAD_MUTEX_INITIALIZER;
static bool s_stoppingSignal = false;

static bool isTriggered = false;
static pthread_mutex_t mutex_isTriggered = PTHREAD_MUTEX_INITIALIZER;


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

static bool toggleStreamingOption()
{
    bool x;
    pthread_mutex_lock(&mutex_isTriggered);
    {
        isTriggered = !isTriggered;
        x = isTriggered;
    }
    pthread_mutex_unlock(&mutex_isTriggered);
    return x;
}
static void isCameraTriggered()
{
    bool x;
    pthread_mutex_lock(&mutex_isTriggered);
    {
        x = isTriggered;
    }
    pthread_mutex_unlock(&mutex_isTriggered); 
    return x;
}

void *streamListenerThread(void *args)
{
    while (!isStoppingSignalReceived()) {
        if (isCameraTriggered()) {
            PIRState state = MotionSensor_getState();
            if (state == PIR_DETECT) {
                if (!Stream_isLive()) {
                    Stream_toggle();
                }
                Utilities_sleepForMs(STREAM_DURATION_MS);
            }
        }
        Utilities_sleepForMs(SLEEP_FREQUENCY_MS);
    }

    return NULL;
}

// ***************************** PUBLIC ****************************** //


void Stream_Controller_start(void) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&s_streamThreadId, &attr, streamListenerThread, NULL);
}

bool Stream_Controller_toggle(void)
{
    return toggleStreamingOption();
}

// returns true if the camera is currently set to triggered otherwise false
bool Stream_Controller_isTriggered(void)
{
    return isCameraTriggered();
}


void Stream_Controller_stop(void) {
    sendStoppingSignal();
    pthread_join(s_streamThreadId, NULL);
}