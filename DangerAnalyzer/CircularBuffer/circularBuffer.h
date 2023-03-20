/**
 * The Circular Buffer module stores the detection records for the motion sensor
 * which is used to calculate the frequency of detections over a period of time
 */
#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include <stdbool.h>
#include <stdlib.h>

#define BUFFER_SIZE 100


typedef struct {
    unsigned int numTriggers;
    long long timestampMs;
} Record;

typedef struct {
    Record buf[BUFFER_SIZE];
    size_t next;
    bool isFull;
} History;


void Buffer_make(History *buffer);
void Buffer_add(History *buffer, int numDetections);
double Buffer_frequency(const History *buffer);


#endif