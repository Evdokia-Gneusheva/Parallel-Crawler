#include <vector>
#include <mutex>
#include <shared_mutex>
#include <list>
#include <iostream>
#include "crawler.h"
#include "hash_table.h"
#include "webpage.h"
#include "tests.h"


int main()
{
    manualTestingHash();

    static const char *urls[] = {
        "http://www.microsoft.com",
        "http://www.yahoo.com",
        "http://www.wikipedia.org",
        "http://slashdot.org"};

    int num_urls =  4;
    int max_wait = 30 * 1000; /* Wait max. 30 seconds */
    StripedHashSet<Webpage> HashSet(10);

    //crawl_webpage(urls, num_urls, max_wait, HashSet);

    TestingLikeTutorial();

    
}