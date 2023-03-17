#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>

#include "ledMfa.h"

#include "../Utilities/utilities.h"
#include "../Timer/timer.h"

#define LED0_BRIGHTNESS         "/sys/class/leds/beaglebone:green:usr0/brightness"
#define LED1_BRIGHTNESS         "/sys/class/leds/beaglebone:green:usr1/brightness"
#define LED2_BRIGHTNESS         "/sys/class/leds/beaglebone:green:usr2/brightness"
#define LED3_BRIGHTNESS         "/sys/class/leds/beaglebone:green:usr3/brightness"

#define ON                      1
#define OFF                     0

#define SLEEP_MS                1000
#define REFRESH_MFA_FREQ_MS     30000

static Timer timer;
static char currentCode[MFA_PASSWORD_LIMIT] = {0};


static void *mainloop(void *args);
static bool s_stop = false;
static pthread_t s_thread;
static pthread_mutex_t s_stopMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_codeMutex = PTHREAD_MUTEX_INITIALIZER;

// ------------------------- PRIVATE ------------------------- //

static bool receivedExitSignal() 
{
    bool signalReceived = false;
    pthread_mutex_lock(&s_stopMutex);
    {
        signalReceived = s_stop;
    }
    pthread_mutex_unlock(&s_stopMutex);
    return signalReceived;
}

static void sendExitSignal() 
{
    pthread_mutex_lock(&s_stopMutex);
    {
        s_stop = true;
    }
    pthread_mutex_unlock(&s_stopMutex);
}

static void displayCode(const char *code)
{
    Utilities_writeIntValueToFile(code[0], LED0_BRIGHTNESS);
    Utilities_writeIntValueToFile(code[1], LED1_BRIGHTNESS);
    Utilities_writeIntValueToFile(code[2], LED2_BRIGHTNESS);
    Utilities_writeIntValueToFile(code[3], LED3_BRIGHTNESS);
}

static void setCurrentCode(const char *code)
{
    pthread_mutex_lock(&s_codeMutex);
    {
        for (int i = 0; i < MFA_PASSWORD_LIMIT; i++) {
            currentCode[i] = code[i];
        }
    }
    pthread_mutex_unlock(&s_codeMutex);
}
static void getCurrentCode(char *code)
{
    pthread_mutex_lock(&s_codeMutex);
    {
        for (int i = 0; i < MFA_PASSWORD_LIMIT; i++) {
            code[i] = currentCode[i];
        }
    }
    pthread_mutex_unlock(&s_codeMutex);
}
static char getRandomBit()
{
    return rand() % 2 ? '1' : '0';
}
static bool isValid(char ch)
{
    return ch == '0' || ch == '1';
}

static void *mainloop(void *args)
{
    char code[MFA_PASSWORD_LIMIT];
    while (!receivedExitSignal()) {
        if (Timer_isExpired(&timer)) {
            Timer_start(REFRESH_MFA_FREQ_MS, &timer);

            code[0] = getRandomBit();
            code[1] = getRandomBit();
            code[2] = getRandomBit();
            code[3] = getRandomBit();

            displayCode(code);
            setCurrentCode(code);
        }
        Utilities_sleepForMs(SLEEP_MS);
    }
    return NULL;
}

// ------------------------- PUBLIC ------------------------- //

void Mfa_start(void)
{
    Timer_start(0, &timer);
    srand(time(NULL));
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&s_thread, &attr, mainloop, NULL);
}
void Mfa_stop(void)
{
    sendExitSignal();
    pthread_join(s_thread, NULL);
}
bool Mfa_isValid(char *codeSequence, size_t size)
{
    char code[MFA_PASSWORD_LIMIT];
    getCurrentCode(code);
    int idx = 0;
    for (int i = 0; i < size; i++) {
        char ch = codeSequence[i];
        if (isspace(ch)) {
            continue;
        }
        if (!isValid(ch)) {
            return false;
        }
        if (idx >= MFA_PASSWORD_LIMIT) {
            return false;
        }
        if (codeSequence[i] != code[idx]) {
            return false;
        }
        idx++;
    }
    if (idx < MFA_PASSWORD_LIMIT) {
        return false;
    }
    return true;
}

