#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "serverNet.h"

#include "../Networking/networking.h"
#include "../LEDMFA/ledMfa.h"
#include "../Settings/settings.h"
#include "../EventLogger/logger.h"
#include "../DangerAnalyzer/dangerAnalyzer.h"

#define WHITESPACE                              " \n\r\t"

#define COMMAND_STOP                            "stop"
#define COMMAND_GET_DANGER_LEVEL                "gdanger"
#define COMMAND_GET_NUM_TRIGGERS                "gtrigger"
#define COMMAND_LOGIN                           "login"
#define COMMAND_TOGGLE                          "toggle"
#define COMMAND_SET_DANGER_THRESHOLD            "sdanger"
#define COMMAND_RESET_DANGER_THRESHOLD          "rdthresh"
#define COMMAND_RESET_DANGER_LEVEL              "rdlevel"
#define COMMAND_CONFIGURE                       "conf"
#define ARG2_CONF_AUTO_LOGOUT                   "autologout"
#define ARG2_CONF_REMOTE_ACCESS                 "remoteaccess"
#define ARG3_CONF_AL_AFTER_30_SEC               "short"
#define ARG3_CONF_AL_AFTER_1_MIN                "medium"
#define ARG3_CONF_AL_AFTER_2_MIN                "long"
#define ARG3_CONF_RA_ENABLE_MFA                 "enmfa"
#define ARG3_CONF_RA_DISABLE_MFA                "nomfa"
#define ARG3_CONF_RA_DISABLE_REMOTE_ACCESS      "noremote"

#define DEVICE_CAMERA                           "cam"
#define DEVICE_LOGGER                           "log"
#define DEVICE_MOTION_SENSOR                    "pir"

#define DANGER_THRESHOLD_MIN                    0
#define DANGER_THRESHOLD_MAX                    99

typedef enum {
    SIGNAL_NONE,
    SIGNAL_STOP,
} Signal;


static bool parseNumber(const char *number, int *result, char *response);
static void getDangerLevel(char *response);
static void getNumTriggers(char *response);
static void loginMFA(void);
static void login(char *response);
static void toggle(char *response);
static void resetDangerThreshold(char *response);
static void resetDangerLevel(char *response);
static void setDangerThreshold(char *response);
static void configureSettings(char *response);
static void configureAutoLogout(char *response);
static void configureRemoteAccess(char *response);
static Signal execute(char *command, char *response);


static bool parseNumber(const char *number, int *result, char *response)
{
    char *end;
    long num = strtol(number, &end, 10);

    // error handling from https://en.cppreference.com/w/c/string/byte/strtol
    if (number == end) {
        // not in the correct format
        snprintf(response, RESPONSE_PACKET_SIZE, "not a number\n");
        return false;
    }
    if (errno == ERANGE || (num > INT_MAX || num < INT_MIN)) {
        // range error
        snprintf(response, RESPONSE_PACKET_SIZE, "number must be between %d and %d\n", 
            INT_MIN, INT_MAX);
        return false;
    }

    *result = (int)num;
    return true;
}


static void getDangerLevel(char *response)
{
    float dangerLevel = DangerAnalyzer_getDangerLevel();
    snprintf(response, RESPONSE_PACKET_SIZE, "Current danger level: %f\n", dangerLevel);
}
static void getNumTriggers(char *response)
{
    int numTriggers = 0;
    // motionSensorController.getNumTriggers();
    snprintf(response, RESPONSE_PACKET_SIZE, "# of triggers recorded: %d\n", numTriggers);
}

static void loginMFA()
{
    #define MFA_PROMPT "Enter MFA code displayed on LEDs: "
    ServerNet_send(MFA_PROMPT, sizeof(MFA_PROMPT));

    char password[RELAY_PACKET_SIZE];
    ServerNet_receive(password);

    if (!Mfa_isValid(password, strlen(password))) {
        ServerNet_send(STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD));
    } else {
        ServerNet_send(STATUS_CODE_OK, sizeof(STATUS_CODE_OK));    
    }
}
static void login(char *response)
{
    #define PASSWORD_PROMPT "Enter password for user admin: "
    ServerNet_send(PASSWORD_PROMPT, sizeof(PASSWORD_PROMPT));

    char password[RELAY_PACKET_SIZE];
    ServerNet_receive(password);
    password[strcspn(password, "\n")] = 0;

    if (!Settings_passwordIsValid(password)) {
        snprintf(response, RESPONSE_PACKET_SIZE, STATUS_CODE_BAD);
        return;
    }
    
    if (Settings_getRemoteAccessPolicySetting() == SETTINGS_RA_ENABLE_MFA) {
        loginMFA();
    } else {
        snprintf(response, RESPONSE_PACKET_SIZE, STATUS_CODE_OK);
    }
}

static void toggle(char *response)
{
    char *nextArg = strtok(NULL, WHITESPACE);

    if (nextArg == NULL) {
        // incorrect number of arguments
        snprintf(response, RESPONSE_PACKET_SIZE,
            "Expected 1 argument.\n"
            "\nUsage:\n"
            "Available devices:\n"
            "\t-" DEVICE_MOTION_SENSOR "\n"
            "\t-" DEVICE_CAMERA "\n"
            "\t-" DEVICE_LOGGER "\n"
        );
        return;
    }

    bool isOn = false;
    if (strncmp(nextArg, DEVICE_CAMERA, sizeof(DEVICE_CAMERA)) == 0) {
        // camera.toggle();
        snprintf(response, RESPONSE_PACKET_SIZE, "camera now is %s\n", isOn ? "on": "off");

    } else if (strncmp(nextArg, DEVICE_LOGGER, sizeof(DEVICE_LOGGER)) == 0) {
        isOn = Logger_toggle();
        snprintf(response, RESPONSE_PACKET_SIZE, "logger now is %s\n", isOn ? "on": "off");

    } else if (strncmp(nextArg, DEVICE_MOTION_SENSOR, sizeof(DEVICE_MOTION_SENSOR)) == 0) {
        // motionSensorController.toggle();
        snprintf(response, RESPONSE_PACKET_SIZE, "motion sensor now is %s\n", isOn ? "on": "off");

    } else {
        snprintf(response, RESPONSE_PACKET_SIZE, 
            "Unrecognized device: '%s'\n"
            "\nUsage:\n"
            "Available devices:\n"
            "\t-" DEVICE_MOTION_SENSOR "\n"
            "\t-" DEVICE_CAMERA "\n"
            "\t-" DEVICE_LOGGER "\n", 
            nextArg
        );
    }
}
static void resetDangerThreshold(char *response)
{
    Settings_setDangerThresholdSetting(DANGER_ANALYZER_DEFAULT_THRESHOLD);
    snprintf(response, RESPONSE_PACKET_SIZE, "danger threshold reset to factory default: %d\n", 
        DANGER_ANALYZER_DEFAULT_THRESHOLD);
}
static void resetDangerLevel(char *response)
{
    DangerAnalyzer_resetDangerLevel();
    snprintf(response, RESPONSE_PACKET_SIZE, "danger level reset to 0");
}
static void setDangerThreshold(char *response)
{
    char *nextArg = strtok(NULL, WHITESPACE);

    if (nextArg == NULL) {
        // incorrect number of arguments
        snprintf(response, RESPONSE_PACKET_SIZE, "Expected 1 argument.\n");
        return;
    }

    int dangerThreshold = 0;
    if (!parseNumber(nextArg, &dangerThreshold, response)) {
        return;
    }

    if (dangerThreshold < DANGER_THRESHOLD_MIN || dangerThreshold > DANGER_THRESHOLD_MAX) {
        snprintf(response, RESPONSE_PACKET_SIZE, "Threshold out of bounds. Expected between %d and %d\n", 
            DANGER_THRESHOLD_MIN, DANGER_THRESHOLD_MAX);
        return;
    }

    Settings_setDangerThresholdSetting(dangerThreshold);
    snprintf(response, RESPONSE_PACKET_SIZE, "Danger threshold set to: %d\n", dangerThreshold);
}
static void configureSettings(char *response)
{
    char *nextArg = strtok(NULL, WHITESPACE);

    if (nextArg == NULL) {
        // incorrect number of arguments
        snprintf(response, RESPONSE_PACKET_SIZE, 
            "Expected 3 arguments.\n"
            "\nUsage:\n"
            "Available settings are:\n"
            "\t- " ARG2_CONF_AUTO_LOGOUT "\n"
            "\t- " ARG2_CONF_REMOTE_ACCESS "\n"
        );
        return;
    }

    if (strncmp(nextArg, ARG2_CONF_AUTO_LOGOUT, sizeof(ARG2_CONF_AUTO_LOGOUT)) == 0) {
        configureAutoLogout(response);
    } else if (strncmp(nextArg, ARG2_CONF_REMOTE_ACCESS, sizeof(ARG2_CONF_REMOTE_ACCESS)) == 0) {
        configureRemoteAccess(response);
    } else {
        snprintf(response, RESPONSE_PACKET_SIZE, 
            "Unrecognized setting '%s'\n"
            "\nUsage:\n"
            "Available settings are:\n"
            "\t- " ARG2_CONF_AUTO_LOGOUT "\n"
            "\t- " ARG2_CONF_REMOTE_ACCESS "\n", 
            nextArg
        );
    }
}
static void configureAutoLogout(char *response)
{
    char *nextArg = strtok(NULL, WHITESPACE);

    if (nextArg == NULL) {
        // incorrect number of arguments
        snprintf(response, RESPONSE_PACKET_SIZE, 
            "Expected 3 arguments.\n"
            "\nUsage:\n"
            "Available policies are:\n"
            "\t- " ARG3_CONF_AL_AFTER_30_SEC "\n"
            "\t- " ARG3_CONF_AL_AFTER_1_MIN "\n"
            "\t- " ARG3_CONF_AL_AFTER_2_MIN "\n"
        );
        return;
    }

    if (strncmp(nextArg, ARG3_CONF_AL_AFTER_30_SEC, sizeof(ARG3_CONF_AL_AFTER_30_SEC)) == 0) {
        Settings_setAutoLogoutSetting(SETTINGS_AL_AFTER_30_SEC);
        snprintf(response, RESPONSE_PACKET_SIZE, "system configured to logout after 30 seconds.");

    } else if (strncmp(nextArg, ARG3_CONF_AL_AFTER_1_MIN, sizeof(ARG3_CONF_AL_AFTER_1_MIN)) == 0) {
        Settings_setAutoLogoutSetting(SETTINGS_AL_AFTER_1_MIN);
        snprintf(response, RESPONSE_PACKET_SIZE, "system configured to logout after 1 min.");

    } else if (strncmp(nextArg, ARG3_CONF_AL_AFTER_2_MIN, sizeof(ARG3_CONF_AL_AFTER_2_MIN)) == 0) {
        Settings_setAutoLogoutSetting(SETTINGS_AL_AFTER_2_MIN);
        snprintf(response, RESPONSE_PACKET_SIZE, "system configured to logout after 2 mins.");

    } else {
        snprintf(response, RESPONSE_PACKET_SIZE, 
            "Unrecognized policy '%s'.\n"
            "\nUsage:\n"
            "Available policies are:\n"
            "\t- " ARG3_CONF_AL_AFTER_30_SEC "\n"
            "\t- " ARG3_CONF_AL_AFTER_1_MIN "\n"
            "\t- " ARG3_CONF_AL_AFTER_2_MIN "\n", 
            nextArg
        );
    }
}
static void configureRemoteAccess(char *response)
{
    char *nextArg = strtok(NULL, WHITESPACE);

    if (nextArg == NULL) {
        // incorrect number of arguments
        snprintf(response, RESPONSE_PACKET_SIZE, 
            "Expected 3 arguments.\n"
            "\nUsage:\n"
            "Available policies are:\n"
            "\t- " ARG3_CONF_RA_ENABLE_MFA "\n"
            "\t- " ARG3_CONF_RA_DISABLE_MFA "\n"
            "\t- " ARG3_CONF_RA_DISABLE_REMOTE_ACCESS "\n"
        );
        return;
    }

    if (strncmp(nextArg, ARG3_CONF_RA_ENABLE_MFA, sizeof(ARG3_CONF_RA_ENABLE_MFA)) == 0) {
        Settings_setRemoteAccessPolicySetting(SETTINGS_RA_ENABLE_MFA);
        snprintf(response, RESPONSE_PACKET_SIZE, "system MFA authentication enabled.");

    } else if (strncmp(nextArg, ARG3_CONF_RA_DISABLE_MFA, sizeof(ARG3_CONF_RA_DISABLE_MFA)) == 0) {
        Settings_setRemoteAccessPolicySetting(SETTINGS_RA_DISABLE_MFA);
        snprintf(response, RESPONSE_PACKET_SIZE, "system MFA authentication disabled.");

    } else if (strncmp(nextArg, ARG3_CONF_RA_DISABLE_REMOTE_ACCESS, 
        sizeof(ARG3_CONF_RA_DISABLE_REMOTE_ACCESS)) == 0) 
    {
        Settings_setRemoteAccessPolicySetting(SETTINGS_RA_DISABLE_REMOTE_ACCESS);
        snprintf(response, RESPONSE_PACKET_SIZE, "system remote access disabled.");

    } else {
        snprintf(response, RESPONSE_PACKET_SIZE, 
            "Unrecogized policy '%s'.\n"
            "\nUsage:\n"
            "Available policies are:\n"
            "\t- " ARG3_CONF_RA_ENABLE_MFA "\n"
            "\t- " ARG3_CONF_RA_DISABLE_MFA "\n"
            "\t- " ARG3_CONF_RA_DISABLE_REMOTE_ACCESS "\n", 
            nextArg
        );
    }
}

static Signal execute(char *command, char *response)
{
    // getting the first part of the command
    char *cmd = strtok(command, WHITESPACE);

    // check if the commands are recognized by the client
    if (cmd == NULL) {
        return SIGNAL_NONE;
    }
    if (strncmp(cmd, COMMAND_LOGIN, sizeof(COMMAND_LOGIN)) == 0) {
        login(response);

    } else if (strncmp(cmd, COMMAND_STOP, sizeof(COMMAND_STOP)) == 0) {
        snprintf(response, RESPONSE_PACKET_SIZE, "stopped.\n");
        return SIGNAL_STOP;

    } else if (strncmp(cmd, COMMAND_GET_DANGER_LEVEL, sizeof(COMMAND_GET_DANGER_LEVEL)) == 0) {
        getDangerLevel(response);

    } else if (strncmp(cmd, COMMAND_GET_NUM_TRIGGERS, sizeof(COMMAND_GET_NUM_TRIGGERS)) == 0) {
        getNumTriggers(response);

    } else if (strncmp(cmd, COMMAND_TOGGLE, sizeof(COMMAND_TOGGLE)) == 0) {
        toggle(response);

    } else if (strncmp(cmd, COMMAND_SET_DANGER_THRESHOLD, sizeof(COMMAND_SET_DANGER_THRESHOLD)) == 0) {
        setDangerThreshold(response);

    } else if (strncmp(cmd, COMMAND_RESET_DANGER_THRESHOLD, sizeof(COMMAND_RESET_DANGER_THRESHOLD)) == 0) {
        resetDangerThreshold(response);

    } else if (strncmp(cmd, COMMAND_RESET_DANGER_LEVEL, sizeof(COMMAND_RESET_DANGER_LEVEL)) == 0) {
        resetDangerLevel(response);

    } else if (strncmp(cmd, COMMAND_CONFIGURE, sizeof(COMMAND_CONFIGURE)) == 0) {
        configureSettings(response);

    } else {
        snprintf(response, RESPONSE_PACKET_SIZE, "Unrecognized command '%s'\n", cmd);
    }
    return SIGNAL_NONE;
}


void ResponseHandler_start(void)
{
    ServerNet_init();

    char command[RELAY_PACKET_SIZE];
    char response[RESPONSE_PACKET_SIZE];

    Signal signal = SIGNAL_NONE;
    while (signal != SIGNAL_STOP) {
        ServerNet_receive(command);
        signal = execute(command, response);
        ServerNet_send(response, strlen(response));
    }

    ServerNet_cleanup();
}

int main(int argc, char **argv)
{
    printf("starting server\n");
    ResponseHandler_start();
    return 0;
}

