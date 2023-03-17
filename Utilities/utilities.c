#include "utilities.h"

#define READ                                "r"
#define WRITE                               "w"

#define EXPORT_PATH                         "/sys/class/gpio/export"
#define GPIO_SETUP_DELAY_MS                 300

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

void Utilities_exportGpioPin(const char *pFilePath, int gpioNumber)
{
    if (!Utililties_isFileExists(pFilePath)) {
        Utilities_writeIntValueToFile(gpioNumber, EXPORT_PATH);
        Utilities_sleepForMs(GPIO_SETUP_DELAY_MS);
    }
}

int Utilities_readGpioValue(const char *pFilePath)
{
    FILE *f = fopen(pFilePath, "r");
    if (f == NULL) {
        fprintf(stderr, "Error opening '%s'.\n", pFilePath);
        exit(1);
    }

    int number = 0;
    if (fscanf(f, "%d", &number) == EOF) { 
        fprintf(stderr, "ERROR READING DATA");
        fclose(f);
        exit(1);
    }

    fclose(f);
    return number;
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

    int charWritten = fputs(value, pFile);
    if (charWritten <= 0) {
        printf("ERROR WRITING DATA");
        fclose(pFile);
        exit(1);
    }
    fclose(pFile);
}