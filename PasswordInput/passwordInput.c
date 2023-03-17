#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "passwordInput.h"


#include "../Joystick/joystick.h"
// #include "../Menu/MockObjects/joystick.h"

#define PASSWORD_LIMIT      10

// ------------------------- PRIVATE ------------------------- //

typedef struct {
    JoystickInput sequence[PASSWORD_LIMIT];
    size_t size;
} Sequence;

static Sequence password;
static Sequence sequence;

static bool isPasswordValid()
{
    bool isValid = (password.size == sequence.size);
    for (int i = 0; i < password.size; i++) {
        isValid = isValid && (password.sequence[i] == sequence.sequence[i]);
    }
    return isValid;
}

static bool Sequence_isFull(const Sequence *seq)
{
    return seq->size == PASSWORD_LIMIT;
}

// precondition: seq is not full
static void Sequence_add(Sequence *seq, JoystickInput input)
{
    seq->sequence[seq->size] = input;
    seq->size++;
}

static void Sequence_reset(Sequence *seq)
{
    seq->size = 0;
}


// ------------------------- PUBLIC ------------------------- //

void PasswordInput_init(void)
{
    // todo: read the password from a file
    Sequence_add(&password, JOYSTICK_UP);
    Sequence_add(&password, JOYSTICK_RIGHT);
    Sequence_add(&password, JOYSTICK_DOWN);
    Sequence_add(&password, JOYSTICK_LEFT);
    Sequence_add(&password, JOYSTICK_CENTER);
}
void PasswordInput_cleanup(void)
{
    // do nothing
}

PInputState PasswordInput_sendNext(JoystickInput input)
{
    if (input == JOYSTICK_NONE) {
        // ignore none inputs
        return P_INPUT_CONTINUE;
    }

    if (Sequence_isFull(&sequence)) {
        Sequence_reset(&sequence);
        return P_INPUT_TOO_LONG;
    }

    Sequence_add(&sequence, input);

    if (input == JOYSTICK_CENTER) {
        PInputState state = isPasswordValid() ? P_INPUT_MATCH : P_INPUT_NO_MATCH;
        Sequence_reset(&sequence);
        return state;
    }

    return P_INPUT_CONTINUE;
}

