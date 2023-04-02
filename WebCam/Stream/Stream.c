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
static pthread_mutex_t mutex_isActive = PTHREAD_MUTEX_INITIALIZER;

// ***************************** PRIVATE ****************************** //

static bool isCameraActive() 
{
    bool x;
    pthread_mutex_lock(&mutex_isActive);
    {
        x = isActive;
    }
    pthread_mutex_unlock(&mutex_isActive);
    return x;
}
static void toggleCameraActive() 
{
    pthread_mutex_lock(&mutex_isActive);
    {
        isActive = !isActive;
    }
    pthread_mutex_unlock(&mutex_isActive);
}

void static killStream(pid_t pid)
{
    char cmdEndStream[32];
    snprintf(cmdEndStream, sizeof(cmdEndStream), END_STREAM_COMMAND, pid);
    Utilities_runCommand(cmdEndStream);
}

static void setCamera(bool enable)
{
    toggleCameraActive();

    if (enable) {
        Timer_start(STARTUP_DELAY_MS, &timer);
        streamingPipe = good_popen(STREAM_COMMAND, "r", &streamingPid);

    } else {
        killStream(streamingPid);
    }
}

// ***************************** PUBLIC ****************************** //

void Stream_init(void)
{
    // nothing
}
void Stream_cleanup(void) 
{
    // nothing
    while (!Timer_isExpired()) {
        Utilities_sleepForMs(1000);
    }

    if (isCameraActive()) {
        setCamera(false);
    }
}

StreamingToggle Stream_toggle(void)
{
    StreamingToggle result;
    bool isOn = isCameraActive();

    // ignore commands
    if (isOn && !Timer_isExpired(&timer)) {
        result.isOperationSucceeded = false;
        return result;
    }

    setCamera(!isOn);

    result.isOperationSucceeded = true;
    result.isActive = !isOn;
    return result;
}

bool Stream_isLive(void)
{
    return isCameraActive();
}
