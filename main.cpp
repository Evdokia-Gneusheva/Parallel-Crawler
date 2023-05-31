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
    //manualTestingHash();
    std::cout << "hello, how are asddasyou?" << std::endl;

    std::vector<std::string> urls;
    urls.push_back("http://www.microsoft.com");
    urls.push_back("http://slashdot.org");
    urls.push_back("http://www.wikipedia.org");
    urls.push_back("http://www.yahoo.com");

    int num_urls =  4;
    int max_wait = 30 * 1000; /* Wait max. 30 seconds */
    StripedHashSet<Webpage> HashSet(10);

    crawl_webpage(urls, num_urls, max_wait, HashSet);

    //TestingLikeTutorial();

    int number = HashSet.get_count();
    std::cout << number << std::endl;
    
}