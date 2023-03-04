#include "utilities.h"

#define READ "r"
#define WRITE "w"

const int SECONDS_TO_MILLISECONDS = 1000;
const int NANO_SECONDS_TO_MILLISECONDS = 1000000;
const int NANO_SECONDS_PER_SECOND = 1000000000;

const int WAIT_LOW = 500;
const int WAIT_HIGH = 3500;

void Utilities_sleepForMs(long double delayInMs) {
    const long double NS_PER_MS = SECONDS_TO_MILLISECONDS * SECONDS_TO_MILLISECONDS;
    const long long NS_PER_SECOND = NANO_SECONDS_PER_SECOND;

    long long delayNs = delayInMs * NS_PER_MS;

    double seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

bool Utililties_isFileExists(const char *pFilePath)
{
    FILE *f = fopen(pFilePath, "r");
    if (f == NULL) {
        return false;
    }
    fclose(f);
    return true;
}

int Utilities_readNumberFromFile(const char* filePath) {
    FILE* file = fopen(filePath, READ);
    if (!file) {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf(" Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }

    int a2dReading = 0;
    int itemsRead = fscanf(file, "%d", &a2dReading);
    if (itemsRead <= 0) {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }

    fclose(file);
    return a2dReading;
}

void Utilities_writeIntValueToFile(int value, const char* path) {
    FILE *pFile = fopen(path, WRITE);
    if (pFile == NULL) {
        printf("ERROR: Unable to open file to write int.\n");
        exit(1);
    }

    fprintf(pFile, "%d", value);

    fclose(pFile);
}

void Utilities_runCommand(char *command) {
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, READ);
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[UTILITIES_BUFFER_SIZE];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL) break;
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}

void Utilities_writeStringValueToFile(const char* value, const char* path) {
    FILE *pFile = fopen(path, WRITE);
    if (pFile == NULL) {
        printf("ERROR: Unable to open file to write string.\n");
        exit(1);
    }

    int charWritten = fprintf(pFile, value);
    if (charWritten <= 0) {
        printf("ERROR WRITING DATA");
        exit(1);
    }
}