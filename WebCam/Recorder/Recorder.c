#include <stdio.h>
#include "../../Utilities/utilities.h"

#define BUFFER_SIZE 1024
/*
record video
./capture -F -c 300 -o > output.raw

convert to mp4
ffmpeg -i output.raw -vcodec copy output.mp4
*/
void Recorder_startRecording(int numFrames, char* fileName) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "./capture -F -c %d -o > %s", numFrames, fileName);
    Utilities_runCommand(buffer);
}
