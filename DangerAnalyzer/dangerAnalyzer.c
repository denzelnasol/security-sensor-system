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

// the sleep should be about 1s because the motion sensor is debounced by the hardware for 2s
// setting it to 1s will always catch detections
#define SLEEP_MS                    1000

static long long timeSinceLastDetectedInMs = 0;
static History detectionHistory;

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

// piecewise linear heuristic for the frequency curve
static double PLfreq(double x)
{
    // maximum detection frequency is 0.5 detections/sec due to hardware debouncing
    #define PLFREQ_PARAM_X1      0.5
    #define PLFREQ_PARAM_Y1      1

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
    // drop by 10% in 5 min
    #define PLLLTS_PARAM_X1      300000
    #define PLLLTS_PARAM_Y1      0.1

    if (x < PLLLTS_PARAM_X1) {
        return x * PLLLTS_PARAM_Y1 / PLLLTS_PARAM_X1;
    }
    return PLLLTS_PARAM_Y1;
}
static double computeFrequencyContribution()
{
    // window size of 5 is 5 samples which is ~= 10s
    #define SPIKE_FREQUENCY_WINDOW  5
    #define SMOOTH_FREQ_WEIGHT      0.1
    #define SPIKE_FREQ_WEIGHT       0.9
    double freq = Buffer_frequency(&detectionHistory);
    double spikeFreq = Buffer_frequencyWindow(&detectionHistory, SPIKE_FREQUENCY_WINDOW);
    return SMOOTH_FREQ_WEIGHT * PLfreq(freq) + SPIKE_FREQ_WEIGHT * PLfreq(spikeFreq);
}
static double computeLTTSContribution()
{
    return PLltts(timeSinceLastDetectedInMs);
}
static double computeBounded(double x)
{
    if (x <= 0) {
        return 0;
    }
    if (x >= 1) {
        return 1;
    }
    return x;
}
static double computeNewDangerLevel(double dLevel)
{
    double freqContribution = computeFrequencyContribution();
    double lttsContribution = computeLTTSContribution();
    double newDLevel = dLevel + freqContribution - lttsContribution;
    return computeBounded(newDLevel);
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


