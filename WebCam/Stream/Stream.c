#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "Stream.h"
#include "Popen.h"

#include "../../Utilities/utilities.h"
#include "../../Timer/timer.h"

#define STREAM_COMMAND      "./capture -F -o -c0 | ffmpeg -i pipe:0 -f mpegts -codec:v mpeg1video -s 640x480 -b:v 4000k -minrate 4000k -maxrate 4000k -bufsize 1835k -muxdelay 0.1 -framerate 30 -bf 0 udp://192.168.7.1:8080"
#define END_STREAM_COMMAND  "pkill -P %d"

#define BUFFER_SIZE         1024

// the camera needs a minimum of 10s to start up otherwise the streaming data will be corrupted
#define STARTUP_DELAY_MS    10000

static FILE *streamingPipe;
static pid_t streamingPid;

static Timer timer;
static bool isActive = false;
static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

// ***************************** PRIVATE ****************************** //

void static killStream(pid_t pid)
{
    char cmdEndStream[32];
    snprintf(cmdEndStream, sizeof(cmdEndStream), END_STREAM_COMMAND, pid);
    Utilities_runCommand(cmdEndStream);
}

// ***************************** PUBLIC ****************************** //

void Stream_init(void)
{
    // nothing
}

void Stream_cleanup(void) 
{
    while (!Timer_isExpired(&timer)) {
        Utilities_sleepForMs(1000);
    }

    if (isActive) {
        killStream(streamingPid);
    }
}

bool Stream_turnOn(void)
{
    pthread_mutex_lock(&s_mutex);
    {
        isActive = true;
        Timer_start(STARTUP_DELAY_MS, &timer);
        streamingPipe = good_popen(STREAM_COMMAND, "r", &streamingPid);
    }
    pthread_mutex_unlock(&s_mutex);
    return true;
}

bool Stream_turnOff(void)
{
    bool res = true;
    pthread_mutex_lock(&s_mutex);
    {
        // allow the camera to terminate gracefully otherwise it will affect future streams
        // making sure that the camera data does not get corrupted
        if (isActive) {
            if (!Timer_isExpired(&timer)) {
                res = false;
            } else {
                isActive = false;
                killStream(streamingPid);
            }
        }
    }
    pthread_mutex_unlock(&s_mutex);
    return res;
}


