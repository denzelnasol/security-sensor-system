#include "buzzer.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "../Utilities/utilities.h"

#define PWM_FILE_PATH_PERIOD        "/dev/bone/pwm/0/a/period"
#define PWM_FILE_PATH_DUTY_CYCLE    "/dev/bone/pwm/0/a/duty_cycle"
#define PWM_FILE_PATH_ENABLE        "/dev/bone/pwm/0/a/enable"

#define CONFIG_P9_22_PIN_PWM        "config-pin p9_22 pwm"


typedef struct {
    int periodInNS;
    int dutyCycleInNS;
    bool isEnabled;
} BuzzerSettings;

static pthread_mutex_t s_buzzerMutex = PTHREAD_MUTEX_INITIALIZER;
static bool isActive = false;

// ------------------------- PRIVATE ------------------------- //

static void setPWMFile(BuzzerSettings info) {
    pthread_mutex_lock(&s_buzzerMutex);
    {
        Utilities_writeIntValueToFile(info.periodInNS, PWM_FILE_PATH_PERIOD);
        Utilities_writeIntValueToFile(info.dutyCycleInNS, PWM_FILE_PATH_DUTY_CYCLE);
        Utilities_writeIntValueToFile(info.isEnabled ? 1 : 0, PWM_FILE_PATH_ENABLE);
        isActive = info.isEnabled;
    }
    pthread_mutex_unlock(&s_buzzerMutex);
}

static bool isAlarmActive()
{
    bool x;
    pthread_mutex_lock(&s_buzzerMutex);
    {
        x = isActive;
    }
    pthread_mutex_unlock(&s_buzzerMutex);
    return x;
}

static void initializeBuzzerSettings() 
{
    BuzzerSettings buzzer = {0};
    buzzer.isEnabled = false;
    setPWMFile(buzzer);
}

// ------------------------- PUBLIC ------------------------- //

void Buzzer_init() 
{
    Utilities_runCommand(CONFIG_P9_22_PIN_PWM);
    initializeBuzzerSettings();
}

void Buzzer_alarm() 
{
    if (!isAlarmActive()) {
        BuzzerSettings buzzer;
        buzzer.isEnabled = true;
        buzzer.dutyCycleInNS = 500000;
        buzzer.periodInNS = 1000000;
        setPWMFile(buzzer);
    }
}
void Buzzer_stopAlarm()
{
    if (isAlarmActive()) {
        BuzzerSettings buzzer = {0};
        buzzer.isEnabled = false;
        setPWMFile(buzzer);
    }
}

void Buzzer_cleanup() 
{
    Buzzer_stopAlarm();
}
