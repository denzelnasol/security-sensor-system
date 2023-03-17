#include <stdbool.h>
#include <pthread.h>


#include "dangerAnalyzer.h"

#define FREQ_WEIGHT                 0.15
#define LTTS_WEIGHT                 0.8
#define COUNT_WEIGHT                0.05

#define HISTORY_WEIGHT              0.8

#define DANGER_THRESHOLD_DEFAULT    0.75

static void *mainloop(void* arg);
static pthread_t s_thread;
static bool s_stop = false;
static pthread_mutex_t s_stopMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t s_dangerThresholdMutex = PTHREAD_MUTEX_INITIALIZER;
static int dangerThreshold = DANGER_THRESHOLD_DEFAULT;

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

static double getDangerThreshold()
{
    double dThreshold = 0;
    pthread_mutex_lock(&s_dangerThresholdMutex);
    {
        dThreshold = dangerThreshold;
    }
    pthread_mutex_unlock(&s_dangerThresholdMutex);
    return dThreshold;
}

static double computeFrequencyContribution(double freq)
{
    // todo
    return 0;
}
static double computeLTTSContribution(double llts)
{
    return 0;
}
static double computeTrCountContribution(double count)
{
    return 0;
}

static double computeNewDangerLevel(double dLevel, double trFreq, double ltts, int trCount)
{
    //todoooooo
    double freqContribution = computeFrequencyContribution(trFreq);
    double lttsContribution = computeLTTSContribution(ltts);
    double trCountContribution = computeTrCountContribution(trCount);

    double newDLevel = FREQ_WEIGHT * freqContribution + LTTS_WEIGHT * lttsContribution +
        COUNT_WEIGHT * trCountContribution;

    return HISTORY_WEIGHT * dLevel + (1 - HISTORY_WEIGHT) * newDLevel;
}

static void *mainloop(void *args)
{
    while (!isStoppingSignalReceived()) {
        // double triggerFreq = motion.getFrequency();
        // double lastTriggerTime = motion.getTimeSinceLastTriggerInSeconds();
        // int triggerCount = motion.getNumTriggers()
        double triggerFreq = 0;
        double lastTriggerTime = 0;
        int triggerCount = 0;

        // compute new danger level
        double level = 0;
        pthread_mutex_lock(&s_dangerLevelMutex);
        {
            level = computeNewDangerLevel(dangerLevel, triggerFreq, lastTriggerTime, triggerCount);
            dangerLevel = level;
        }
        pthread_mutex_unlock(&s_dangerLevelMutex);

        if (level > getDangerThreshold()) {
            // buzzer.soundAlarm();
            // 
        }
    }
    return NULL;
}


// ------------------------- PUBLIC ------------------------- //

void DangerAnalyzer_start(void)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&s_thread, &attr, mainloop, NULL);
}

void DangerAnalyzer_setThreshold(int dangerThresholdPercent)
{
    pthread_mutex_lock(&s_dangerThresholdMutex);
    {
        dangerThreshold = dangerThresholdPercent / 100;
    }
    pthread_mutex_unlock(&s_dangerThresholdMutex);
}
void DangerAnalyzer_resetThreshold(void)
{
    pthread_mutex_lock(&s_dangerThresholdMutex);
    {
        dangerThreshold = DANGER_THRESHOLD_DEFAULT;
    }
    pthread_mutex_unlock(&s_dangerThresholdMutex);
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


