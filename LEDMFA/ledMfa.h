#ifndef LED_MFA_H_
#define LED_MFA_H_

#include <stdlib.h>
#include <stdbool.h>

/**
 * This component uses the 4 LEDs to display a MFA code for remote login 
*/

#define MFA_PASSWORD_LIMIT  4

void Mfa_start(void);
void Mfa_stop(void);

// the code sequence is a list of 0s and 1s
bool Mfa_isValid(char *codeSequence, size_t size);

#endif