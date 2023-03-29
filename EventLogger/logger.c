#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "logger.h"

#define LOG_PATH                "/var/log/thebbg/events.log"

#define LOG_INFO                "INFO"
#define LOG_WARNING             "WARNING"
#define LOG_DANGER              "DANGER"

#define LINE_BUFFER_SIZE        256
#define TIMESTAMP_BUFFER_SIZE   32

typedef enum {
    INFO,
    WARNING,
    DANGER,
} LogLevel;


// ------------------------- PRIVATE ------------------------- //

static bool isEnabled = true;

static pthread_mutex_t s_fmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_toggleMutex = PTHREAD_MUTEX_INITIALIZER;


static bool isActivated()
{
    bool isTurnOn = false;
    pthread_mutex_lock(&s_toggleMutex);
    {
        isTurnOn = isEnabled;
    }
    pthread_mutex_unlock(&s_toggleMutex);
    return isTurnOn;
}

// source: https://stackoverflow.com/questions/1442116/how-to-get-the-date-and-time-values-in-a-c-program
static void getTimeStamp(char *timeStampBuffer, size_t size)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(timeStampBuffer, size, "%d-%02d-%02d %02d:%02d:%02d", 
        tm.tm_year + 1900, 
        tm.tm_mon + 1, 
        tm.tm_mday, 
        tm.tm_hour, 
        tm.tm_min, 
        tm.tm_sec
    );
}

static void clear()
{
    FILE *pFile = fopen(LOG_PATH, "w");
    if (pFile == NULL) {
        fprintf(stderr, "logger: Error opening '%s'.\n", LOG_PATH);
        return;
    }
    char timestamp[TIMESTAMP_BUFFER_SIZE];
    getTimeStamp(timestamp, sizeof(timestamp));
    fprintf(pFile, "[%s] " LOG_INFO " Logs cleared by admin.\n", timestamp);
    fclose(pFile);
}
static void dump()
{
    FILE *pFile = fopen(LOG_PATH, "r");
    if (pFile == NULL) {
        fprintf(stderr, "logger: Error opening '%s'.\n", LOG_PATH);
        return;
    }

    // reading line by line -> source: https://en.cppreference.com/w/c/io/fgets
    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), pFile) != NULL) {
        printf("%s", line);
    }

    fclose(pFile);
}
static void writeMessage(const char *message, LogLevel level)
{
    FILE *pFile = fopen(LOG_PATH, "a");
    if (pFile == NULL) {
        fprintf(stderr, "logger: Error opening '%s'.\n", LOG_PATH);
        return;
    }

    char timestamp[TIMESTAMP_BUFFER_SIZE];
    getTimeStamp(timestamp, sizeof(timestamp));

    switch (level) {
        case INFO:
            fprintf(pFile, "[%s] " LOG_INFO " %s\n", timestamp, message);
            break;
        case WARNING:
            fprintf(pFile, "[%s] " LOG_WARNING " %s\n", timestamp, message);
            break;
        case DANGER:
            fprintf(pFile, "[%s] " LOG_DANGER " %s\n", timestamp, message);
            break;
        default:
            assert(false);
    }

    fclose(pFile);
}


// ------------------------- PUBLIC ------------------------- //

void Logger_logInfo(const char *message)
{
    if (!isActivated()) {
        return;
    }
    pthread_mutex_lock(&s_fmutex);
    {
        writeMessage(message, INFO);
    }
    pthread_mutex_unlock(&s_fmutex);
}
void Logger_logWarning(const char *message)
{
    if (!isActivated()) {
        return;
    }
    pthread_mutex_lock(&s_fmutex);
    {
        writeMessage(message, WARNING);
    }
    pthread_mutex_unlock(&s_fmutex);
}
void Logger_logError(const char *message)
{
    if (!isActivated()) {
        return;
    }
    pthread_mutex_lock(&s_fmutex);
    {
        writeMessage(message, DANGER);
    }
    pthread_mutex_unlock(&s_fmutex);
}
void Logger_dump(void)
{
    pthread_mutex_lock(&s_fmutex);
    {
        dump();
    }
    pthread_mutex_unlock(&s_fmutex);
}
void Logger_clearLogs(void)
{
    pthread_mutex_lock(&s_fmutex);
    {
        clear();
    }
    pthread_mutex_unlock(&s_fmutex);
}
bool Logger_toggle(void)
{
    bool isTurnOn = false;
    pthread_mutex_lock(&s_toggleMutex);
    {
        isEnabled = !isEnabled;
        isTurnOn = isEnabled;
    }
    pthread_mutex_unlock(&s_toggleMutex);
    return isTurnOn;
}
bool Logger_isEnabled(void)
{
    return isActivated();
}

