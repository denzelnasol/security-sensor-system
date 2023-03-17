#ifndef TIMER_H_
#define TIMER_H_

#include <stdbool.h>
#include <time.h>

/**
 * The Timer module is used to aid in the debouncing of the accelerometer.
*/
typedef struct {
    struct timespec end;
} Timer;

/**
 * Starts the timer
 * Call Timer_setTimeout before starting the timer
*/
void Timer_start(long milliseconds, Timer *timer);

// Returns true if the timer has expired, false otherwise
bool Timer_isExpired(const Timer *timer);


#endif
