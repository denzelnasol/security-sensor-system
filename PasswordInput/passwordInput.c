#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>


#include "passwordInput.h"
#include "../Joystick/joystick.h"
#include "../Utilities/utilities.h"

#define PASSWORD_LIMIT      10
#define SLEEP_FREQUENCY_MS  10

// ------------------------- PRIVATE ------------------------- //

static JoystickInput password[PASSWORD_LIMIT];
static PInputSequence inputSequence = {.input = NULL, .size = 0};

void *getPasswordThread(void *args);
static pthread_t s_passwordThreadId;

void *getPasswordThread(void *args)
{
    int index = 0;
    JoystickInput input = JOYSTICK_NONE;
    while (index < PASSWORD_LIMIT) {
        input = Joystick_getPressed();
        if (input == JOYSTICK_CENTER) {
            break;
        }
        if (input != JOYSTICK_NONE) {

            printf("%d\n", input);

            password[index] = input;
            index++;
        }
        Utilities_sleepForMs(SLEEP_FREQUENCY_MS);
    }
    inputSequence.input = password;
    inputSequence.size = index;
    return NULL;
}

// ------------------------- PUBLIC ------------------------- //

PInputSequence PasswordInput_getInputSequence(void)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&s_passwordThreadId, &attr, getPasswordThread, NULL);
    pthread_join(s_passwordThreadId, NULL);
    return inputSequence;
}

