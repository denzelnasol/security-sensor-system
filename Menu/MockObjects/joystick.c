#include <stdio.h>
#include <stdlib.h>

#include "joystick.h"

static JoystickInput charToInput(char input)
{
    switch (input)
    {
    case '1':
        return JOYSTICK_UP;
    case '2':
        return JOYSTICK_RIGHT;    
    case '3':
        return JOYSTICK_DOWN;    
    case '4':
        return JOYSTICK_LEFT;
    case '5':
        return JOYSTICK_CENTER;
    default:
        // printf("ignoring unrecognized character: %c\n", input);
        break;
    }
    return JOYSTICK_NONE;
}

void Joystick_init(void)
{
    printf("dont call joystick init!\n");
    exit(EXIT_FAILURE);
}
void Joystick_cleanup(void)
{
    printf("dont call joystick cleanup!\n");
    exit(EXIT_FAILURE);
}

// handles debouncing
JoystickInput Joystick_getPressed(void)
{
    char input = '1';
    scanf("%c", &input);
    return charToInput(input);
}
