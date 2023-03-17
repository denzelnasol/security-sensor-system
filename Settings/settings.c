#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "settings.h"

#define SETTING_PATH            "./settings.txt"

// ------------------------- PRIVATE ------------------------- //

static SettingsALOpt autoLogoutSetting;
static SettingsRAOpt remoteAccessSetting;
static int dangerThresholdSetting;

static pthread_mutex_t s_almutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_ramutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_dtmutex = PTHREAD_MUTEX_INITIALIZER;


// ------------------------- PUBLIC ------------------------- //

void Settings_init(void)
{
    FILE *pFile = fopen(SETTING_PATH, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file to write string.\n");
        return;
    }

    if (fscanf(pFile, "%d %d %d", 
        (int *)&autoLogoutSetting, (int *)&remoteAccessSetting, &dangerThresholdSetting) == EOF) 
    { 
        fprintf(stderr, "ERROR READING DATA");
        fclose(pFile);
        return;
    }
    fclose(pFile);
}
void Settings_cleanup(void)
{
    // save all settings to file
    FILE *pFile = fopen(SETTING_PATH, "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file to write string.\n");
        return;
    }

    fprintf(pFile, "%d %d %d", 
        (int)autoLogoutSetting, (int)remoteAccessSetting, dangerThresholdSetting);
    fclose(pFile);
}

SettingsALOpt Settings_getAutoLogoutSetting(void)
{
    SettingsALOpt opt;
    pthread_mutex_lock(&s_almutex);
    {
        opt = autoLogoutSetting;
    }
    pthread_mutex_unlock(&s_almutex);
    return opt;
}
void Settings_setAutoLogoutSetting(SettingsALOpt setting)
{
    pthread_mutex_lock(&s_almutex);
    {
        autoLogoutSetting = setting;
    }
    pthread_mutex_unlock(&s_almutex);
}

SettingsRAOpt Settings_getRemoteAccessPolicySetting(void)
{
    SettingsRAOpt opt;
    pthread_mutex_lock(&s_ramutex);
    {
        opt = remoteAccessSetting;
    }
    pthread_mutex_unlock(&s_ramutex);
    return opt;
}
void Settings_setRemoteAccessPolicySetting(SettingsRAOpt setting)
{
    pthread_mutex_lock(&s_ramutex);
    {
        remoteAccessSetting = setting;
    }
    pthread_mutex_unlock(&s_ramutex);
}

int Settings_getDangerThresholdSetting(void)
{
    int opt;
    pthread_mutex_lock(&s_dtmutex);
    {
        opt = dangerThresholdSetting;
    }
    pthread_mutex_unlock(&s_dtmutex);
    return opt;
}
void Settings_setDangerThresholdSetting(int newThreshold)
{
    pthread_mutex_lock(&s_dtmutex);
    {
        dangerThresholdSetting = newThreshold;
    }
    pthread_mutex_unlock(&s_dtmutex);
}
