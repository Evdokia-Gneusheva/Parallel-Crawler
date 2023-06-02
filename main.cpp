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

    std::set<std::string> urls;
    urls.insert("http://www.microsoft.com");
    urls.insert("http://slashdot.org");
    urls.insert("http://www.wikipedia.org");
    urls.insert("http://www.yahoo.com");

    int num_urls =  4;
    int max_wait = 30 * 1000; /* Wait max. 30 seconds */
    StripedHashSet<Webpage> HashSet(10);

    crawl_webpage(urls, num_urls, max_wait, HashSet);

    //TestingLikeTutorial();

    int number = HashSet.get_count();
    std::cout << number << std::endl;
    
}