#ifndef EVENT_LOGGER_H_
#define EVENT_LOGGER_H_

#include <stdbool.h>

/**
 * The event logger module will log events to the log
*/

void Logger_logInfo(const char *message);
void Logger_logWarning(const char *message);
void Logger_logError(const char *message);

void Logger_dump(void);
void Logger_clearLogs(void);

// returns true if logging is ON. false otherwise
bool Logger_toggle(void);
bool Logger_isEnabled(void);

#endif