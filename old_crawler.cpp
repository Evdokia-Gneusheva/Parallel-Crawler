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
#include <unordered_set>



struct CompareFirst {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);

        return h1;  
    }
};


struct WriteFunctionData
{
    std::string url;
    std::string parent;
    StripedHashSet<Webpage> *HashSet;
    int current_level;
    bool only_local;
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *checked_urls;
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *new_urls;
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

std::unordered_set<std::pair<std::string, std::string>, CompareFirst> extract_links(const std::string &receivedData, std::unordered_set<std::pair<std::string, std::string>, CompareFirst> &checked_urls, std::string root_url, bool only_local)
{
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> links;
    std::regex link_pattern("<a[^>]*href=\"([^\"]*)\"[^>]*>");

    std::string root_domain = extract_domain(root_url);


    for (std::sregex_iterator it(receivedData.begin(), receivedData.end(), link_pattern), end_it; it != end_it; ++it)
    {   
        std::string url = (*it)[1].str();

        if (url.find("http") == std::string::npos && url.find("https") == std::string::npos && url[0] == '/' && url[1] != '/') {
            url = "http://" + root_domain + url;
        }

        if(url.substr(0, 4) == "www."){
            url = "http://" + url;
        }

        std::string domain = extract_domain(url);

        
        bool exists = std::any_of(checked_urls.begin(), checked_urls.end(),
        [&](const std::pair<std::string, std::string>& pair) {
            return pair.first == url;
        });

        if (!only_local && !exists && url.find("index.php") == std::string::npos && !domain.empty()) {
            links.insert(std::make_pair(url, root_url));
        }

        if (!domain.empty() && compare_domains(root_domain, domain) && !exists && url.find("index.php") == std::string::npos && only_local) {
            links.insert(std::make_pair(url, root_url));
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
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> links = extract_links(receivedData, *(userData->checked_urls), userData->url, userData->only_local);

    userData->new_urls->insert(links.begin(), links.end());

    std::unordered_set<std::string> onlyLinks;
    for (const auto& pair : links) {
        onlyLinks.insert(pair.first);
    }

    Webpage toInsert(userData->url, onlyLinks, userData->current_level, userData->parent);

    if (userData->HashSet->contains(toInsert))
    {
        userData->HashSet->add_links(toInsert, onlyLinks);
    }
    else
    {
        userData->HashSet->add(toInsert);
    }

    return totalSize;
}

static void init(CURLM *cm, std::string url, std::string parent, StripedHashSet<Webpage> &HashSet, std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *checked_urls, std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *new_urls, bool only_local, int level )
{
    WriteFunctionData *data = new WriteFunctionData;
    data->url = url;
    data->HashSet = &HashSet;
    data->checked_urls = checked_urls;
    data->new_urls = new_urls;
    data->parent = parent;
    data->only_local = only_local;
    data->current_level = level;

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

std::unordered_set<std::pair<std::string, std::string>, CompareFirst> crawl_webpage(std::unordered_set<std::pair<std::string, std::string>, CompareFirst> urls, int num_urls, int max_wait, StripedHashSet<Webpage> &HashSet)
{

    CURLM *cm = NULL;
    CURL *eh = NULL;
    CURLMsg *msg = NULL;
    CURLcode return_code;
    int still_running = 0, msgs_left = 0;
    int http_status_code;
    const char *szUrl;

    int links_at_a_time = 100;
    int max_level = 10;
    bool only_local= true;

    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *checked_urls = new std::unordered_set<std::pair<std::string, std::string>, CompareFirst>();
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *current_urls = new std::unordered_set<std::pair<std::string, std::string>, CompareFirst>(urls);
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *new_urls = new std::unordered_set<std::pair<std::string, std::string>, CompareFirst>();
    
    while(!current_urls->empty()){

    std::cout << "Current level: " << "Beginning of while loop" << std::endl;
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst>::iterator iter = current_urls->begin();
    std::advance(iter, std::min(links_at_a_time, static_cast<int>(current_urls->size()))); 
    checked_urls->insert(current_urls->begin(), iter);

    curl_global_init(CURL_GLOBAL_ALL);

    cm = curl_multi_init();
    int counter = 0;
    int current_level = 1;
    for (std::unordered_set<std::pair<std::string, std::string>, CompareFirst>::iterator it = current_urls->begin(); it != current_urls->end(); ++it)
    {   
        counter++;
        Webpage *parent_webpage = HashSet.get(it->second);
        if (parent_webpage != NULL)
        {
            current_level = parent_webpage->current_level + 1;
        }

        init(cm, it->first, it->second, HashSet, checked_urls, new_urls, only_local, current_level);
        
        if(counter == links_at_a_time || current_level > max_level){
            break;
        }
    }
    std::cout << "Current level: " << current_level << std::endl;
    if(current_level > max_level){
        break;
    }


    curl_multi_perform(cm, &still_running);
    do
    {
        int numfds = 0;
        int res = curl_multi_wait(cm, NULL, 0, max_wait, &numfds);
        if (res != CURLM_OK)
        {
            fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
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
                //printf("200 OK for %s\n", szUrl);
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

    iter = current_urls->begin();
    std::advance(iter, std::min(links_at_a_time, static_cast<int>(current_urls->size()))); 
    current_urls->erase(current_urls->begin(), iter);
    current_urls->insert(new_urls->begin(), new_urls->end());

    new_urls->clear();


    }

    return urls;
}