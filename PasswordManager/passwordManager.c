#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "passwordManager.h"

#include "../Utilities/utilities.h"
#include "../Share/host.h"

#define PASSWORD_PATH           "/var/lib/" HOST_NAME "/password.txt"
#define MS_PASSWORD_PATH        "/var/lib/" HOST_NAME "/mssecret.txt"


// ------------------------- PRIVATE ------------------------- //

typedef struct {
    // the string is ended with 0
    const char *cstring;
    size_t len;
} string;

static char remoteAdminPasswordRaw[PWMGR_PASSWORD_LIMIT];
static char menuSystemPasswordRaw[PWMGR_MSPASSWORD_LIMIT];

static string remoteAdminPassword;
static string menuSystemPassword;

static pthread_mutex_t s_passwordMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_patternMutex = PTHREAD_MUTEX_INITIALIZER;

static bool isPasswordCorrect(const string *correctPassword, const string *password)
{
    int minlen = (int)correctPassword->len;
    if ((int)password->len < minlen) {
        minlen = (int)password->len;
    }
    bool isMatch = (correctPassword->len == password->len);
    for (int i = 0; i < minlen; i++) {
        isMatch = isMatch && (correctPassword->cstring[i] == password->cstring[i]);
    }
    return isMatch;
}
static bool isValidMSChar(char ch)
{
    return ch >= '1' && ch <= '4';
}
static bool isMenuSystemPasswordValid(const string *password)
{
    for (int i = 0; i < password->len; i++) {
        if (!isValidMSChar(password->cstring[i])) {
            return false;
        }
    }
    return true;
}

// ------------------------- PUBLIC ------------------------- //

void PasswordManager_init(void)
{
    Utilities_readStringFromFile(PASSWORD_PATH, remoteAdminPasswordRaw, PWMGR_PASSWORD_LIMIT);
    Utilities_readStringFromFile(MS_PASSWORD_PATH, menuSystemPasswordRaw, PWMGR_PASSWORD_LIMIT);
    remoteAdminPassword.cstring = remoteAdminPasswordRaw;
    remoteAdminPassword.len = strlen(remoteAdminPasswordRaw);
    menuSystemPassword.cstring = menuSystemPasswordRaw;
    menuSystemPassword.len = strlen(menuSystemPasswordRaw);
}
void PasswordManager_cleanup(void)
{
    Utilities_writeStringValueToFile(remoteAdminPassword.cstring, PASSWORD_PATH);
    Utilities_writeStringValueToFile(menuSystemPassword.cstring, MS_PASSWORD_PATH);
}

bool PasswordManager_isLoginPasswordCorrect(const char *password, size_t len)
{
    string passwordStr;
    passwordStr.cstring = password;
    passwordStr.len = len;

    bool res = false;
    pthread_mutex_lock(&s_passwordMutex);
    {
        res = isPasswordCorrect(&remoteAdminPassword, &passwordStr);
    }
    pthread_mutex_unlock(&s_passwordMutex);
    return res;
}
bool PasswordManager_isMenuSystemPasswordCorrect(const char *password, size_t len)
{
    string passwordStr;
    passwordStr.cstring = password;
    passwordStr.len = len;

    bool res = false;
    pthread_mutex_lock(&s_patternMutex);
    {
        res = isPasswordCorrect(&menuSystemPassword, &passwordStr);
    }
    pthread_mutex_unlock(&s_patternMutex);
    return res;
}

bool PasswordManager_changeLoginPassword(const char *newPassword, size_t len)
{
    pthread_mutex_lock(&s_passwordMutex);
    {
        snprintf(remoteAdminPasswordRaw, PWMGR_PASSWORD_LIMIT, "%s", newPassword);
        remoteAdminPassword.len = len;
    }
    pthread_mutex_unlock(&s_passwordMutex);
    return true;
}
bool PasswordManager_changeMenuSystemPassword(const char *newPassword, size_t len)
{
    string passwordStr;
    passwordStr.cstring = newPassword;
    passwordStr.len = len;

    if (!isMenuSystemPasswordValid(&passwordStr)) {
        return false;
    }

    pthread_mutex_lock(&s_patternMutex);
    {
        snprintf(menuSystemPasswordRaw, PWMGR_MSPASSWORD_LIMIT, "%s", newPassword);
        menuSystemPassword.len = len;
    }
    pthread_mutex_unlock(&s_patternMutex);
    return true;
}
