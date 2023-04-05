#ifndef PASSWORD_MANAGER_H_
#define PASSWORD_MANAGER_H_

#include <stdbool.h>
#include <stdlib.h>

#define PWMGR_PASSWORD_LIMIT    32
#define PWMGR_MSPASSWORD_LIMIT  8

void PasswordManager_init(void);
void PasswordManager_cleanup(void);

// all passwords need to be < PWMGR_PASSWORD_LIMIT
// all menu system passwords need to by < PWMGR_MSPASSWORD_LIMIT

bool PasswordManager_isLoginPasswordCorrect(char *password);
bool PasswordManager_isMenuSystemPasswordCorrect(char *password);

bool PasswordManager_changeLoginPassword(char *newPassword);
bool PasswordManager_changeMenuSystemPassword(char *newPassword);

#endif