#include <time.h>
#include <stdlib.h>

#include "timer.h"

#define NS_PER_MS           1000000
#define MS_PER_SECONDS      1000

// ------------------------- PRIVATE ------------------------- //

// By StackOverflow: https://stackoverflow.com/questions/15024623/convert-milliseconds-to-timespec-for-gnu-port
static void addMilliseconds(struct timespec *pTime, long milliseconds) 
{
    pTime->tv_sec += milliseconds / MS_PER_SECONDS;
    pTime->tv_nsec += (milliseconds % MS_PER_SECONDS) * NS_PER_MS;
}

static bool isBefore(const struct timespec *pTimeFirst, const struct timespec *pTimeSecond) 
{
    if (pTimeFirst->tv_sec == pTimeSecond->tv_sec) {
        return pTimeFirst->tv_nsec < pTimeSecond->tv_nsec;
    }
    return pTimeFirst->tv_sec < pTimeSecond->tv_sec;
}

// ------------------------- PUBLIC ------------------------- //

void Timer_start(long milliseconds, Timer *timer) 
{
    struct timespec tnow;
    clock_gettime(CLOCK_REALTIME, &tnow);
    addMilliseconds(&tnow, milliseconds);
    timer->end = tnow;
}

bool Timer_isExpired(const Timer *timer) 
{
    struct timespec tnow;
    clock_gettime(CLOCK_REALTIME, &tnow);
    return !isBefore(&tnow, &(timer->end));
}

