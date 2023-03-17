#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    SETTINGS_AL_AFTER_30_SEC,
    SETTINGS_AL_AFTER_1_MIN,
    SETTINGS_AL_AFTER_2_MIN,
} SettingsALOpt;

typedef enum {
    SETTINGS_RA_DISABLE_MFA,
    SETTINGS_RA_ENABLE_MFA,
    SETTINGS_RA_DISABLE_REMOTE_ACCESS,
} SettingsRAOpt;


void Settings_init(void);
void Settings_cleanup(void);

SettingsALOpt Settings_getAutoLogoutSetting(void);
void Settings_setAutoLogoutSetting(SettingsALOpt setting);

SettingsRAOpt Settings_getRemoteAccessPolicySetting(void);
void Settings_setRemoteAccessPolicySetting(SettingsRAOpt setting);

int Settings_getDangerThresholdSetting(void);
void Settings_setDangerThresholdSetting(int newThreshold);

bool Settings_passwordIsValid(const char *password);
void Settings_changePassword(const char *newPassword, size_t size);

#endif