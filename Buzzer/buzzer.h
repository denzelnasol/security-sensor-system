#include <stdbool.h>

#ifndef BUZZER_H
#define BUZZER_H

void Buzzer_start(void);

// Play hit noise
void Buzzer_playHit(void);

// Play miss noise
void Buzzer_playMiss(void);

void Buzzer_stop(void);

#endif
