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

std::string extract_domain(const std::string& url) {
    std::regex url_pattern("(http|https):\\/\\/([^\\/]*)");
    std::smatch match;
    if (std::regex_search(url, match, url_pattern)) {
        std::string domain = match[2];
        if (domain.substr(0, 4) == "www.") {
            domain = domain.substr(4);
        }
        return domain;
    }
    return "";
}

bool compare_domains(const std::string& domain1, const std::string& domain2) {
    return std::equal(domain1.begin(), domain1.end(), domain2.begin(),
        [](char a, char b) {
            return std::tolower(a) == std::tolower(b);
        }
    );
}

std::set<std::string> extract_links(const std::string &receivedData, std::set<std::string> &checked_urls, std::string root_url)
{
    std::set<std::string> links;
    std::regex link_pattern("<a[^>]*href=\"([^\"]*)\"[^>]*>");

    std::string root_domain = extract_domain(root_url);

    for (std::sregex_iterator it(receivedData.begin(), receivedData.end(), link_pattern), end_it; it != end_it; ++it)
    {
        std::string url = (*it)[1].str();
        std::string domain = extract_domain(url);

        if (url.find("http") == std::string::npos && url.find("https") == std::string::npos) {
            url = root_url + url;
        }
        
        if (!domain.empty() && compare_domains(root_domain, domain) && checked_urls.find(url) == checked_urls.end()) {
            links.insert(url);
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
    std::set<std::string> links = extract_links(receivedData, *(userData->checked_urls), userData->url);

    userData->new_urls->insert(links.begin(), links.end());
    Webpage toInsert(userData->url, links, userData->current_level);

    if (userData->HashSet->contains(toInsert))
    {
        // std::cout << "Already contains: " << userData->url << std::endl;
        userData->HashSet->add_links(toInsert, links);
        // std::cout << userData->url << std::endl;
    }
    else
    {
        // std::cout << "Does not contain: " << userData->url << std::endl;
        userData->HashSet->add(toInsert);
        // std::cout << userData->url << std::endl;
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
    int still_running = 0, msgs_left = 0;
    int http_status_code;
    const char *szUrl;

    std::set<std::string> *checked_urls = new std::set<std::string>();
    std::set<std::string> *current_urls = new std::set<std::string>(urls);
    std::set<std::string> *new_urls = new std::set<std::string>();

    while(!current_urls->empty()){

    curl_global_init(CURL_GLOBAL_ALL);

    cm = curl_multi_init();

    for (std::set<std::string>::iterator it = current_urls->begin(); it != current_urls->end(); ++it)
    {
        init(cm, *it, HashSet, checked_urls, new_urls);
    }

    curl_multi_perform(cm, &still_running);
    std::cout << "Crawling..." << std::endl;
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
    std::cout << "Finished crawling" << std::endl;
    // Move contents of current_urls to checked_urls
    checked_urls->insert(current_urls->begin(), current_urls->end());

    // Move contents of new_urls to current_urls
    current_urls->clear();
    current_urls->insert(new_urls->begin(), new_urls->end());

    // Make new_urls empty
    new_urls->clear();


    // Print current_urls
    std::cout << "\nCurrent URLs:" << std::endl;
    for (const auto &url : *current_urls)
    {
        std::cout << url << std::endl;
    }

    }

    return urls;
}