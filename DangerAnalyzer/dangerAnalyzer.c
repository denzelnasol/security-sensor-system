#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "dangerAnalyzer.h"
#include "CircularBuffer/circularBuffer.h"

#include "../Settings/settings.h"
#include "../MotionSensor/motionSensor.h"
#include "../Utilities/utilities.h"
#include "../Timer/timer.h"

#define FREQ_WEIGHT                 200
#define LTTS_WEIGHT                 0.8

#define HISTORY_WEIGHT              0.8

#define DANGER_THRESHOLD_DEFAULT    0.75
#define SLEEP_MS                    1000

static long long timeSinceLastDetectedInMs = 0;
static History detectionHistory;
static double avgTriggerFreq = 0;

static void *mainloop(void* arg);
static pthread_t s_thread;
static bool s_stop = false;
static pthread_mutex_t s_stopMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t s_dangerLevelMutex = PTHREAD_MUTEX_INITIALIZER;
static double dangerLevel = 0;

// ------------------------- PRIVATE ------------------------- //

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

static double getDangerLevel()
{
    double level = 0;
    pthread_mutex_lock(&s_dangerLevelMutex);
    {
        level = dangerLevel;
    }
    pthread_mutex_unlock(&s_dangerLevelMutex);
    return level;
}
static void setDangerLevel(double level)
{
    pthread_mutex_lock(&s_dangerLevelMutex);
    {
        dangerLevel = level;
    }
    pthread_mutex_unlock(&s_dangerLevelMutex);
}
static double getDangerThreshold()
{
    int thresholdPercent = Settings_getDangerThresholdSetting();
    return ((double)thresholdPercent) / 100;
}

// piecewise linear function for the frequency curve
static double PLfreq(double x)
{
    // the endpoints
    #define PLFREQ_PARAM_X1      0.5
    #define PLFREQ_PARAM_Y1      1

    // the intersection point
    // increase by 10% after 1 detection in 30s period
    #define PLFREQ_PARAM_PX0     0.033
    #define PLFREQ_PARAM_PY0     0.1

    if (x < PLFREQ_PARAM_PX0) {
        return x * PLFREQ_PARAM_PY0 / PLFREQ_PARAM_PX0;
    }
    double slope = (PLFREQ_PARAM_Y1 - PLFREQ_PARAM_PY0) / (PLFREQ_PARAM_X1 - PLFREQ_PARAM_PX0);
    return slope * (x - PLFREQ_PARAM_X1) + PLFREQ_PARAM_Y1;
}
static double PLltts(double x)
{
    // drop by 5% in 5 min
    #define PLLLTS_PARAM_X1      300000
    #define PLLLTS_PARAM_Y1      0.05

    if (x < PLLLTS_PARAM_X1) {
        return x * PLLLTS_PARAM_Y1 / PLLLTS_PARAM_X1;
    }
    return PLLLTS_PARAM_Y1;
}
static double computeFrequencyContribution()
{
    double freq = Buffer_frequency(&detectionHistory);
    return PLfreq(freq);
}
static double computeLTTSContribution()
{
    return PLltts(timeSinceLastDetectedInMs);
}

static double computeNewDangerLevel(double dLevel)
{
    double freqContribution = computeFrequencyContribution();
    double lttsContribution = computeLTTSContribution();
    double newDLevel = dLevel + freqContribution - lttsContribution;
    return newDLevel > 1 ? 1 : newDLevel;
}

static void *mainloop(void *args)
{
    timeSinceLastDetectedInMs = Timer_timestampInMs();
    while (!isStoppingSignalReceived()) {
        int numDetections = 0;
        if (MotionSensor_getState() == PIR_DETECT) {
            timeSinceLastDetectedInMs = Timer_timestampInMs() - timeSinceLastDetectedInMs;
            numDetections++;
        }
        Buffer_add(&detectionHistory, numDetections);

        // compute new danger level
        double level = getDangerLevel();
        level = computeNewDangerLevel(level);
        setDangerLevel(level);

        if (level > getDangerThreshold()) {
            // buzzer.soundAlarm();
            // 
        }
        Utilities_sleepForMs(SLEEP_MS);
    }
    return NULL;
}


// ------------------------- PUBLIC ------------------------- //

void DangerAnalyzer_start(void)
{
    Buffer_make(&detectionHistory);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&s_thread, &attr, mainloop, NULL);
}

void DangerAnalyzer_resetDangerLevel(void)
{
    pthread_mutex_lock(&s_dangerLevelMutex);
    {
        dangerLevel = 0;
    }
    pthread_mutex_unlock(&s_dangerLevelMutex);
}

double DangerAnalyzer_getDangerLevel(void)
{
    double level = 0;
    pthread_mutex_lock(&s_dangerLevelMutex);
    {
        level = dangerLevel;
    }
    pthread_mutex_unlock(&s_dangerLevelMutex);
    return level;
}

void DangerAnalyzer_stop(void)
{
    sendStoppingSignal();
    pthread_join(s_thread, NULL);
}


