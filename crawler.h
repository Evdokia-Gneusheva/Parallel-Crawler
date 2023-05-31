#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/multi.h>
#include "hash_table.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

struct WriteFunctionData {
    std::string url;
    StripedHashSet<Webpage> *HashSet;
};


static size_t cb(char* data, size_t size, size_t nmemb, void* userdata) {

    size_t totalSize = size * nmemb;

    // Copy the received data to the dynamically allocated array
    char* receivedData = (char*)malloc(totalSize);
    std::memcpy(receivedData, data, totalSize);

    // Print the received data
    //std::cout << "Received Data: " << receivedData << std::endl;

    WriteFunctionData* userData = (struct WriteFunctionData*)userdata;

    Webpage toInsert(userData->url, receivedData, totalSize);
    
    userData->HashSet->add(toInsert);

    return totalSize;
}

static void init(CURLM *cm, std::string url, StripedHashSet<Webpage> &HashSet)
{   
    WriteFunctionData *data = new WriteFunctionData;
    data->url = url;
    data->HashSet = &HashSet;
    

    CURL *eh = curl_easy_init();

    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, data);
    curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
    curl_easy_setopt(eh, CURLOPT_URL, url.c_str());
    curl_easy_setopt(eh, CURLOPT_PRIVATE, url.c_str());
    curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);

    curl_multi_add_handle(cm, eh);
}



std::vector<std::string> crawl_webpage(std::vector<std::string> urls, int num_urls, int max_wait, StripedHashSet<Webpage> &HashSet)
{

    CURLM *cm = NULL;
    CURL *eh = NULL;
    CURLMsg *msg = NULL;
    CURLcode return_code;
    int still_running = 0, i = 0, msgs_left = 0;
    int http_status_code;
    const char *szUrl;

    curl_global_init(CURL_GLOBAL_ALL);

    cm = curl_multi_init();

    for (i = 0; i < num_urls; ++i)
    {
        init(cm, urls[i], HashSet);
    }
    urls.clear();

    curl_multi_perform(cm, &still_running);

    do
    {
        int numfds = 0;
        int res = curl_multi_wait(cm, NULL, 0, max_wait, &numfds);
        if (res != CURLM_OK)
        {
            fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
            return urls;
        }

        curl_multi_perform(cm, &still_running);

    } while (still_running);

    while ((msg = curl_multi_info_read(cm, &msgs_left)))
    {
        if (msg->msg == CURLMSG_DONE)
        {
            eh = msg->easy_handle;

            return_code = msg->data.result;
            if (return_code != CURLE_OK)
            {
                fprintf(stderr, "CURL error code: %d\n", msg->data.result);
                continue;
            }

            // Get HTTP status code
            http_status_code = 0;
            szUrl = NULL;

            curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_status_code);
            curl_easy_getinfo(eh, CURLINFO_PRIVATE, &szUrl);

            if (http_status_code == 200)
            {
                printf("200 OK for %s\n", szUrl);
            }
            else
            {
                fprintf(stderr, "GET of %s returned http status code %d\n", szUrl, http_status_code);
            }

            curl_multi_remove_handle(cm, eh);
            curl_easy_cleanup(eh);
        }
        else
        {
            fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
        }
    }

    curl_multi_cleanup(cm);

    //Here we will check if newUrls are null, if not we call crawl_webpage recursively 
    //with the new links

    return urls;
}