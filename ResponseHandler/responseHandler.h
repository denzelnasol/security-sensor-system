/**
 * The Response Handler module handles networking tasks for the internal UDP connection.
 * This will run in the main thread and keep blocking until the 'stop' request is received.
*/
#ifndef HANDLER_H_
#define HANDLER_H_

void ResponseHandler_start(void);

#endif