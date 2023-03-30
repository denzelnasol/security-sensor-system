#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "StreamController.h"

#include "../../Utilities/utilities.h"
#include "../../MotionSensor/motionSensor.h"

#define READ "r"
#define WRITE "w"

#define STREAM_COMMAND "./capture -F -o -c0 | ffmpeg -i pipe:0 -f mpegts -codec:v mpeg1video -s 640x480 -b:v 4000k -minrate 4000k -maxrate 4000k -bufsize 1835k -muxdelay 0.1 -framerate 30 -bf 0 udp://192.168.7.1:8080"

#define STREAM_DURATION_MS  120000
#define SLEEP_FREQUENCY_MS  10
#define BUFFER_SIZE         1024

// ------------------------- PRIVATE ------------------------- //

void* streamListenerThread(void* arg);
static pthread_t s_streamThreadId;
static pthread_mutex_t s_stoppingMutex = PTHREAD_MUTEX_INITIALIZER;
static bool s_stoppingSignal = false;

static bool isStreamingOn = false;

static StreamingOption streamingOption = STREAM_TRIGGER;
static pthread_mutex_t s_streamingMutex = PTHREAD_MUTEX_INITIALIZER;

static bool isStoppingSignalReceived()
{
    bool received = false;
    pthread_mutex_lock(&s_stoppingMutex);
    {
        received = s_stoppingSignal;
    }
    pthread_mutex_unlock(&s_stoppingMutex);
    return received;
}
static void sendStoppingSignal()
{
    pthread_mutex_lock(&s_stoppingMutex);
    {
        s_stoppingSignal = true;
    }
    pthread_mutex_unlock(&s_stoppingMutex);
}
static void setStreamingOption(StreamingOption opt)
{
    pthread_mutex_lock(&s_streamingMutex);
    {
        streamingOption = opt;
    }
    pthread_mutex_unlock(&s_streamingMutex);
}
static StreamingOption getStreamingOption()
{
    StreamingOption opt;
    pthread_mutex_lock(&s_streamingMutex);
    {
        opt = streamingOption;
    }
    pthread_mutex_unlock(&s_streamingMutex);
    return opt;
}

void static executeStream(int duration) {
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(STREAM_COMMAND, READ);

    signal(SIGALRM, exit);
    alarm(duration);

    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[BUFFER_SIZE];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL) break;
    }

    alarm(0);

    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", STREAM_COMMAND);
        printf(" exit code: %d\n", exitCode);
    }
}

static void onStreamTrigger()
{
    PIRState state = MotionSensor_getState();
    if (state == PIR_DETECT && !isStreamingOn) {
        printf("Stream Starting\n");
        // start streaming for 15 seconds
        executeStream(STREAM_DURATION_MS);
        isStreamingOn = true;
        Utilities_sleepForMs(STREAM_DURATION_MS);
        printf("STREAM ENDING\n");
        isStreamingOn = false;
    }
}
static void onStreamOff()
{

}
static void onStreamOn()
{

}

void *streamListenerThread(void *args)
{
    while (!isStoppingSignalReceived()) {
        // StreamingOption opt = getStreamingOption();
        // switch (opt) {
        //     case STREAM_OFF:
        //         onStreamOff();
        //         break;
        //     case STREAM_ON:
        //         onStreamOn();
        //         break;
        //     case STREAM_TRIGGER:
        //         onStreamTrigger();
        //         break;
        //     default:
        //         assert(false);
        // }
        onStreamTrigger();
        Utilities_sleepForMs(SLEEP_FREQUENCY_MS);
    }

    return NULL;
}

// ***************************** PUBLIC ****************************** //


void Stream_Controller_start(void) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&s_streamThreadId, &attr, streamListenerThread, NULL);
}

void Stream_Controller_setStreamingOption(StreamingOption opt)
{
    setStreamingOption(opt);
}

void Stream_Controller_stop(void) {
    sendStoppingSignal();
    pthread_join(s_streamThreadId, NULL);
}