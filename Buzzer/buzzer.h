#include <stdbool.h>

#ifndef BUZZER_H
#define BUZZER_H

void Buzzer_init(void);

void Buzzer_alarm(void);
void Buzzer_stopAlarm(void);

void Buzzer_cleanup(void);

#endif
