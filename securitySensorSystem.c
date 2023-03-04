#include <stdio.h>

#include "WebCam/Recorder/Recorder.h"

int main(int arg, char** args)
{
    printf("Hello World!\n");
    Recorder_startRecording(300, "test.raw");
    return 0;
}