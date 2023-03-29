#include <stdio.h>

#include "WebCam/Recorder/Recorder.h"
#include "WebCam/Stream/StreamController.h"
#include "Controller/controller.h"
#include "ResponseHandler/responseHandler.h"

int main(int argc, char **argv)
{
    printf("hello world\n");
    // Recorder_startRecording(300, "test.raw");

    Controller_start();
    
    ResponseHandler_start();

    Controller_stop();

    // PInputSequence seq = PasswordInput_getInputSequence();
    // printf("got\n");
    // for (int i = 0; i < seq.size; i++) {
    //     printf("%d: %d\n", i, seq.input[i]);
    // }
    return 0;
}
