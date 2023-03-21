#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "passwordInput.h"

#include "../PasswordManager/passwordManager.h"
#include "../Joystick/joystick.h"
// #include "../Menu/MockObjects/joystick.h"

// ------------------------- PRIVATE ------------------------- //

typedef struct {
    JoystickInput sequence[PWMGR_MSPASSWORD_LIMIT - 1];
    size_t size;
} Sequence;

static Sequence sequence;

static char inputToChar(JoystickInput input)
{
    switch (input) {
        case JOYSTICK_UP:
            return '1';
        case JOYSTICK_RIGHT:
            return '2';
        case JOYSTICK_DOWN:
            return '3';
        case JOYSTICK_LEFT:
            return '4';
        default:
            assert(false);
    }
    return 0;
}
// precondition: buffer must have size == PWMGR_MSPASSWORD_LIMIT
static void sequenceToString(const Sequence *seq, char *buffer)
{
    for (int i = 0; i < seq->size; i++) {
        buffer[i] = charToInput(seq->sequence[i]);
    }
    buffer[seq->size] = 0;
}
static bool isPasswordValid()
{
    char buffer[PWMGR_MSPASSWORD_LIMIT];
    sequenceToString(&sequence, buffer);
    return PasswordManager_isMenuSystemPasswordCorrect(buffer);
}

static bool Sequence_isFull(const Sequence *seq)
{
    return seq->size == (PWMGR_MSPASSWORD_LIMIT - 1);
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
    // do nothing
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

    if (input == JOYSTICK_CENTER) {
        PInputState state = isPasswordValid() ? P_INPUT_MATCH : P_INPUT_NO_MATCH;
        Sequence_reset(&sequence);
        return state;
    }

    Sequence_add(&sequence, input);

    return P_INPUT_CONTINUE;
}

