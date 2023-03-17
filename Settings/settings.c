#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "settings.h"

#define SETTING_PATH            "./settings.txt"
#define PASSWORD_PATH           "./password.txt"
#define PASSWORD_BUFFER_SIZE    32

// ------------------------- PRIVATE ------------------------- //

static SettingsALOpt autoLogoutSetting;
static SettingsRAOpt remoteAccessSetting;
static int dangerThresholdSetting;
static char remoteAdminPassword[PASSWORD_BUFFER_SIZE] = {0};

static pthread_mutex_t s_almutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_ramutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_dtmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_passwordMutex = PTHREAD_MUTEX_INITIALIZER;

static void getPassword()
{
    FILE *pFile = fopen(PASSWORD_PATH, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file to write string.\n");
        return;
    }
    fgets(remoteAdminPassword, PASSWORD_BUFFER_SIZE, pFile);
    remoteAdminPassword[strcspn(remoteAdminPassword, "\n")] = 0;
    fclose(pFile);
}
static void savePassword()
{
    FILE *pFile = fopen(PASSWORD_PATH, "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file to write string.\n");
        return;
    }
    if (fputs(remoteAdminPassword, pFile) == EOF) {
        printf("ERROR: unable to write to file: password.txt\n");
        fclose(pFile);
        return;
    }
    fclose(pFile);
}
static bool isValid(const char *password)
{
    return strncmp(remoteAdminPassword, password, PASSWORD_BUFFER_SIZE) == 0;
}
static void setPassword(const char *password, size_t size)
{
    size_t len = PASSWORD_BUFFER_SIZE;
    if (size < len) {
        len = size;
    }
    snprintf(remoteAdminPassword, len, "%s", password);
    remoteAdminPassword[strcspn(remoteAdminPassword, "\n")] = 0;
}

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

    // get the password
    getPassword();
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

    // save the password
    savePassword();
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
bool Settings_passwordIsValid(const char *password)
{
    bool res = false;
    pthread_mutex_lock(&s_passwordMutex);
    {
        res = isValid(password);
    }
    pthread_mutex_unlock(&s_passwordMutex);
    return res;
}
void Settings_changePassword(const char *newPassword, size_t size)
{
    pthread_mutex_lock(&s_passwordMutex);
    {
        setPassword(newPassword, size);
    }
    pthread_mutex_unlock(&s_passwordMutex);
}
