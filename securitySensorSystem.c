#include <stdio.h>

#include "WebCam/Recorder/Recorder.h"
#include "WebCam/Stream/StreamController.h"
#include "WebCam/Stream/Stream.h"
#include "Controller/controller.h"
#include "ResponseHandler/responseHandler.h"

int main(int argc, char **argv)
{
    printf("starting...\n");
    // Recorder_startRecording(300, "test.raw");

    Controller_start();
    
    char buffer[32];

    // ResponseHandler_start();
    printf("on\n");
    Stream_toggle();
    fgets(buffer, sizeof(buffer), stdin);

    printf("off\n");
    Stream_toggle();
    fgets(buffer, sizeof(buffer), stdin);

    printf("on\n");
    Stream_toggle();
    fgets(buffer, sizeof(buffer), stdin);

    Controller_stop();

    return 0;
}
