#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "StreamController.h"
#include "openbsdpopen.h"

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
static FILE *streamingPipe;
static pid_t streamingPid;

static bool hasStreamingOptionChanged = false;
static pthread_mutex_t s_hasStreamingOptionChangedMutex = PTHREAD_MUTEX_INITIALIZER;

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
        hasStreamingOptionChanged = (streamingOption != opt);
        streamingOption = opt;
    }
    pthread_mutex_unlock(&s_streamingMutex);
}
static bool isStreamingOptionChanged()
{
    bool x;
    pthread_mutex_lock(&s_streamingMutex);
    {
        x = hasStreamingOptionChanged;
    }
    pthread_mutex_unlock(&s_streamingMutex);
    return x;
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
void static waitForStream(FILE *pipe)
{
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
void static executeStream(int duration) {
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(STREAM_COMMAND, READ);

    signal(SIGALRM, exit);
    alarm(duration);

    waitForStream(pipe);
}
void static killStream(FILE *pipe)
{
    // signal(SIGALRM, exit);
    // alarm(1);

    // pid_t pid = fileno(pipe);

    // kill the process
    int status = kill(streamingPid, SIGTERM);
    if (status == 0) {
        printf("Process %d killed\n", pid);
    } else {
        printf("Failed to kill process %d\n", pid);
    }

    char buffer[BUFFER_SIZE];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL) break;
    }

    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", STREAM_COMMAND);
        printf(" exit code: %d\n", exitCode);
    }
    isStreamingOn = false;
}
void onAlarm(int x)
{
    exit(x);
    killStream(streamingPipe);
}
static void onStreamTrigger()
{
    if (isStreamingOn) {
        killStream(streamingPipe);
    }
    PIRState state = MotionSensor_getState();
    if (state == PIR_DETECT) {
        // start streaming for 15 seconds
        signal(SIGALRM, onAlarm);
        alarm(STREAM_DURATION_MS);

        streamingPipe = popen(STREAM_COMMAND, READ);
        isStreamingOn = true;
    }
}
static void onStreamOff()
{
    if (isStreamingOn) {
        killStream(streamingPipe);
    }
}
static void onStreamOn()
{
    if (isStreamingOn) {
        alarm(0);
    } else {
        streamingPipe = good_popen(STREAM_COMMAND, READ, &streamingPid);
        isStreamingOn = true;
    }
}

void *streamListenerThread(void *args)
{
    while (!isStoppingSignalReceived()) {
        if (isStreamingOptionChanged()) {
            StreamingOption opt = getStreamingOption();
            switch (opt) {
                case STREAM_OFF:
                    onStreamOff();
                    break;
                case STREAM_ON:
                    onStreamOn();
                    break;
                case STREAM_TRIGGER:
                    onStreamTrigger();
                    break;
                default:
                    assert(false);
            }
            hasStreamingOptionChanged = false;
        }
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
StreamingOption Stream_Controller_getStreamingOption(void)
{
    return getStreamingOption();
}

void Stream_Controller_stop(void) {
    sendStoppingSignal();
    pthread_join(s_streamThreadId, NULL);
}