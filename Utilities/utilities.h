/**
 * The Utilities module contains general helper functions which can be used
 * in any modules that they're needed in. Functions include sleepfor x
 * milliseconds, reading/writing from a file, and checking if a file exists.
 */
#ifndef UTILITIES
#define UTILITIES

#include <stdlib.h>
#include <stdbool.h>

void Utilities_sleepForMs(long double delayInMs);

void Utilities_exportGpioPin(const char *pFilePath, int gpioNumber);
int Utilities_readGpioValue(const char *pFilePath);

void Utilities_writeIntValueToFile(int value, const char* path);
void Utilities_writeStringValueToFile(const char* value, const char* path);
bool Utililties_isFileExists(const char* pFilePath);
void Utilities_runCommand(char *command);
void Utilities_readStringFromFile(const char *pFilePath, char *buffer, size_t size);

#endif