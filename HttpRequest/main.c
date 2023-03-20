#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	return size;
}

int main(int argc, char **argv)
{
    curl_global_init(CURL_GLOBAL_ALL);

    CURL *handle;
    CURLcode res;
    
    handle = curl_easy_init();

    if (handle == NULL) {
        printf("is null\n");
        return -1;
    }

    char *data="name=daniel&project=curl";
    // curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(handle, CURLOPT_URL, "http://localhost:8080/level");

    res = curl_easy_perform(handle);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    curl_easy_cleanup(handle);

    return 0;
}
