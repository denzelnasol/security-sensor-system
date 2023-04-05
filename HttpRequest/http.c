#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>

#include "http.h"

#include "../Share/host.h"

// link with -lcurl
// using libcurl4-openssl-dev tools
// to install run: sudo apt-get libcurl4-openssl-dev on the bbg

#define POST_BUFFER_SIZE    64
#define ENDPOINT_URL        "http://" CLOUD_ADDR ":8088/level"

// ------------------------- PRIVATE ------------------------- //

static CURL *httpHandle;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    // dont print to stdout
	return size;
}

// ------------------------- PUBLIC ------------------------- //

void Http_init(void)
{
    curl_global_init(CURL_GLOBAL_ALL);
    httpHandle = curl_easy_init();
    if (httpHandle == NULL) {
        fprintf(stderr, "ERRROR: http init error\n");
        exit(1);
    }
}
bool Http_post(const HttpPostData *data)
{
    curl_easy_reset(httpHandle);

    char buff[POST_BUFFER_SIZE];
    snprintf(buff, sizeof(buff), "dangerLevel=%f", data->dangerLevel);
    curl_easy_setopt(httpHandle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(httpHandle, CURLOPT_POSTFIELDS, buff);
    curl_easy_setopt(httpHandle, CURLOPT_URL, ENDPOINT_URL);

    CURLcode res = curl_easy_perform(httpHandle);
    return res != CURLE_OK;
}
void Http_cleanup(void)
{
    curl_easy_cleanup(httpHandle);
}

