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
#include <thread>
#include <atomic>
#include <semaphore.h>

std::atomic<bool> has_level_been_reached(false);

struct CompareFirst
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
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

std::string extract_domain(const std::string &url)
{
    std::regex url_pattern("(http|https):\\/\\/([^\\/]*)");
    std::smatch match;
    if (std::regex_search(url, match, url_pattern))
    {
        std::string domain = match[2];
        if (domain.substr(0, 4) == "www.")
        {
            domain = domain.substr(4);
        }
        return domain;
    }

    return "";
}

bool compare_domains(const std::string &domain1, const std::string &domain2)
{
    return std::equal(domain1.begin(), domain1.end(), domain2.begin(),
                      [](char a, char b)
                      {
                          return std::tolower(a) == std::tolower(b);
                      });
}

std::unordered_set<std::pair<std::string, std::string>, CompareFirst> extract_links(const std::string &receivedData, std::unordered_set<std::pair<std::string, std::string>, CompareFirst> &checked_urls, std::string root_url, bool only_local)
{
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> links;
    std::regex link_pattern("<a[^>]*href=\"([^\"]*)\"[^>]*>");

    std::string root_domain = extract_domain(root_url);

    for (std::sregex_iterator it(receivedData.begin(), receivedData.end(), link_pattern), end_it; it != end_it; ++it)
    {
        std::string url = (*it)[1].str();

        if (url.find("http") == std::string::npos && url.find("https") == std::string::npos && url[0] == '/' && url[1] != '/')
        {
            url = "http://" + root_domain + url;
        }

        if (url.substr(0, 4) == "www.")
        {
            url = "http://" + url;
        }

        std::string domain = extract_domain(url);

        bool exists = std::any_of(checked_urls.begin(), checked_urls.end(),
                                  [&](const std::pair<std::string, std::string> &pair)
                                  {
                                      return pair.first == url;
                                  });

        if (!only_local && !exists && url.find("index.php") == std::string::npos && !domain.empty())
        {
            links.insert(std::make_pair(url, root_url));
        }

        if (!domain.empty() && compare_domains(root_domain, domain) && !exists && url.find("index.php") == std::string::npos && only_local)
        {
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
    for (const auto &pair : links)
    {
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

static void init(CURLM *cm, std::string url, std::string parent, StripedHashSet<Webpage> &HashSet, std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *checked_urls, std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *new_urls, bool only_local, int level)
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

// This is the function that each thread will run.
void process_links(std::unordered_set<std::pair<std::string, std::string>, CompareFirst> urls, int num_urls, int max_wait, StripedHashSet<Webpage> &HashSet,
                   std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *checked_urls, std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *current_urls,
                   std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *new_urls, bool only_local, int maxlevel)
{
    CURLM *cm = curl_multi_init();
    CURL *eh;
    CURLMsg *msg;
    CURLcode return_code;
    int still_running = 0, msgs_left = 0;
    int http_status_code;
    const char *szUrl;
    int current_level = 1;

    int a = 0;

    for (std::unordered_set<std::pair<std::string, std::string>, CompareFirst>::iterator it = urls.begin(); it != urls.end(); ++it)
    {   
        Webpage *parent_webpage = HashSet.get(it->second);
        if (parent_webpage != NULL)
        {
            current_level = parent_webpage->current_level + 1;
        }

        if(current_level <= maxlevel)
        {
            init(cm, it->first, it->second, HashSet, checked_urls, new_urls, only_local, current_level);

        }
        else{
            has_level_been_reached.store(true);
        }
    }

    // A bunch of curl_multi_* functions to process the added easy handles. (From the documentation)
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

    // Test for completion and report any errors (From the documentation)
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
                // printf("200 OK for %s\n", szUrl);
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
}

void crawl_webpage(std::unordered_set<std::pair<std::string, std::string>, CompareFirst> urls, int num_urls, int max_wait, StripedHashSet<Webpage> &HashSet)
{
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *checked_urls = new std::unordered_set<std::pair<std::string, std::string>, CompareFirst>();
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *current_urls = new std::unordered_set<std::pair<std::string, std::string>, CompareFirst>(urls);
    std::unordered_set<std::pair<std::string, std::string>, CompareFirst> *new_urls = new std::unordered_set<std::pair<std::string, std::string>, CompareFirst>();

    int num_threads = 4; // Number of threads to use

    // Create a vector to hold the thread objects
    std::vector<std::thread> threads;
    int max_level = 5;
    int current_iteration = 0;
    bool only_local = true;
    int max_links_per_thread = 50;


    while (true)
    {
        std::cout << "Iteration: " << current_iteration << std::endl;
        current_iteration++;
        // Add the urls we are about to check to the `checked_urls` set, so we do not treat them as new urls
        checked_urls->insert(current_urls->begin(), current_urls->end());

        // Split the `current_urls` into `num_threads` parts
        int num_per_thread = std::min(static_cast<int>(current_urls->size() / num_threads + 1), max_links_per_thread);

        auto iter = current_urls->begin();

        // Create the threads
        for (int i = 0; i < num_threads; ++i)
        {
            std::cout << "Thread: " << i << "Num per thread" << num_per_thread << std::endl;
            // Determine the range of urls that this thread will process
            auto next_iter = iter;
            std::advance(next_iter, std::min(num_per_thread, static_cast<int>(std::distance(iter, current_urls->end()))));

            // Create a new set for this range of urls
            std::unordered_set<std::pair<std::string, std::string>, CompareFirst> thread_urls(iter, next_iter);

            // Create and start the thread
            threads.push_back(std::thread(process_links, thread_urls, num_urls, max_wait, std::ref(HashSet), checked_urls, current_urls, new_urls, only_local, max_level));

            // Advance the iterator for the next thread
            iter = next_iter;
        }

        // Wait for all threads to finish
        for (std::thread &t : threads)
            t.join();

        threads.clear();
        // Merge the new urls into the current urls
        current_urls->insert(new_urls->begin(), new_urls->end());

        // Clear the new urls
        new_urls->clear();

        for (const auto &pair : *current_urls)
        {
            std::cout << "Current URL: " << pair.first << std::endl;
        }

        for (const auto &pair : *checked_urls)
        {
            std::cout << "Checked URL: " << pair.first << std::endl;
        }

        // If we have reached the maximum level, stop
        if (has_level_been_reached)
        {
            break;
        }
    }
}