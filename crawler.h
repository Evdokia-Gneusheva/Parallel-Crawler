#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/multi.h>
#include "hash_table.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <regex>
#include <set>

struct WriteFunctionData
{
    std::string url;
    StripedHashSet<Webpage> *HashSet;
    int current_level;

    std::set<std::string> *checked_urls;
    std::set<std::string> *new_urls;
};

std::set<std::string> extract_links(char *receivedData, std::set<std::string> *checked_urls)
{
    std::set<std::string> links;
    std::cregex_iterator url_begin(receivedData, receivedData + strlen(receivedData), std::regex("<a\\s+(?:[^>]*?\\s+)?href=\"([^\"]*)\"", std::regex_constants::icase));
    std::cregex_iterator url_end;
    for (std::cregex_iterator i = url_begin; i != url_end; ++i)
    {
        std::cmatch match = *i;
        if (checked_urls->count(match[1].str()))
        {
        }
        else
        {
            links.insert(match[1].str());
        }
    }
    return links;
}

static size_t cb(char *data, size_t size, size_t nmemb, void *userdata)
{
    size_t totalSize = size * nmemb;

    char *receivedData = (char *)malloc(totalSize);
    std::memcpy(receivedData, data, totalSize);

    WriteFunctionData *userData = (struct WriteFunctionData *)userdata;
    std::set<std::string> links = extract_links(receivedData, userData->checked_urls);

    userData->new_urls->insert(links.begin(), links.end());
    Webpage toInsert(userData->url, links, userData->current_level);

    if (userData->HashSet->contains(toInsert))
    {
        userData->HashSet->add_links(toInsert, links);
    }
    else
    {
        userData->HashSet->add(toInsert);
        std::cout << userData->url << std::endl;
    }

    return totalSize;
}

static void init(CURLM *cm, std::string url, StripedHashSet<Webpage> &HashSet, std::set<std::string> *checked_urls, std::set<std::string> *new_urls)
{
    WriteFunctionData *data = new WriteFunctionData;
    data->url = url;
    data->HashSet = &HashSet;
    data->current_level = 1;
    data->checked_urls = checked_urls;
    data->new_urls = new_urls;

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

std::set<std::string> crawl_webpage(std::set<std::string> urls, int num_urls, int max_wait, StripedHashSet<Webpage> &HashSet)
{

    CURLM *cm = NULL;
    CURL *eh = NULL;
    CURLMsg *msg = NULL;
    CURLcode return_code;
    int still_running = 0, i = 0, msgs_left = 0;
    int http_status_code;
    const char *szUrl;

    std::set<std::string> *checked_urls = new std::set<std::string>();
    std::set<std::string> *current_urls = new std::set<std::string>(urls);
    std::set<std::string> *new_urls = new std::set<std::string>();

    curl_global_init(CURL_GLOBAL_ALL);

    cm = curl_multi_init();

    for (std::set<std::string>::iterator it = current_urls->begin(); it != current_urls->end(); ++it)
    {
        init(cm, *it, HashSet, checked_urls, new_urls);
    }

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

    // Move contents of current_urls to checked_urls
    checked_urls->insert(current_urls->begin(), current_urls->end());

    // Move contents of new_urls to current_urls
    current_urls->clear();
    current_urls->insert(new_urls->begin(), new_urls->end());

    // Make new_urls empty
    new_urls->clear();

    // Print checked_urls
    std::cout << "Checked URLs:" << std::endl;
    for (const auto &url : *checked_urls)
    {
        std::cout << url << std::endl;
    }

    // Print current_urls
    std::cout << "\nCurrent URLs:" << std::endl;
    for (const auto &url : *current_urls)
    {
        std::cout << url << std::endl;
    }

    // Print new_urls
    std::cout << "\nNew URLs:" << std::endl;
    for (const auto &url : *new_urls)
    {
        std::cout << url << std::endl;
    }

    return urls;
}