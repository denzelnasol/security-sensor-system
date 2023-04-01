#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

/**
 * This module handles any http request that needs to be performed to the GCP server.
 * 
*/

typedef struct {
    double dangerLevel;
} HttpPostData;

void Http_init(void);
bool Http_post(const HttpPostData *data);
void Http_cleanup(void);

#endif