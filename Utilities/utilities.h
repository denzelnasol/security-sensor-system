/**
 * The Utilities module contains general helper functions which can be used
 * in any modules that they're needed in. Functions include sleepfor x
 * milliseconds, reading/writing from a file, and checking if a file exists.
 */
#ifndef UTILITIES
#define UTILITIES

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define UTILITIES_BUFFER_SIZE 1024

void Utilities_sleepForMs(long double delayInMs);
int Utilities_readNumberFromFile();

void Utilities_writeIntValueToFile(int value, const char* path);
void Utilities_writeStringValueToFile(const char* value, const char* path);
bool Utililties_isFileExists(const char* pFilePath);
void Utilities_runCommand(char *command);

#endif