#include "circularBuffer.h"

#include "../Timer/timer.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MS_PER_S 1000

// ------------------------- PRIVATE ------------------------- //

// precondition: buffer is not empty
static Record getOldestReading(const History *buffer)
{
    if (buffer->isFull) {
        return buffer->buf[(buffer->next + 1) % BUFFER_SIZE];
    } 
    return buffer->buf[0];
}
static Record getLatestReading(const History *buffer)
{
    if (buffer->isFull) {
        return buffer->buf[(BUFFER_SIZE + buffer->next - 1) % BUFFER_SIZE];
    }
    return buffer->buf[buffer->next - 1];
}

// ------------------------- PUBLIC ------------------------- //

void Buffer_make(History *buffer)
{
    buffer->next = 1;
    buffer->isFull = false;
    Record empty = {.numTriggers = 0, .timestampMs = Timer_timestampInMs()};
    buffer->buf[0] = empty;
}
void Buffer_add(History *buffer, int numDetections)
{
    Record newRecord;
    newRecord.numTriggers = getLatestReading(buffer).numTriggers + numDetections;
    newRecord.timestampMs = Timer_timestampInMs();
    buffer->buf[buffer->next] = newRecord;
    buffer->next = (buffer->next + 1) % BUFFER_SIZE;
}
double Buffer_frequency(const History *buffer)
{
    Record r1 = getLatestReading(buffer);
    Record r2 = getOldestReading(buffer);
    if (r1.timestampMs == r2.timestampMs) {
        return 0;
    }
    return ((double)(r1.numTriggers - r2.numTriggers)) / (r1.timestampMs - r2.timestampMs) * MS_PER_S;
}


