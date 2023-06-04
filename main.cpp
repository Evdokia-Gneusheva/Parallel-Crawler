#include <vector>
#include <mutex>
#include <shared_mutex>
#include <list>
#include <iostream>
#include "crawler.h"
#include "hash_table.h"
#include "webpage.h"
#include "tests.h"
#include <chrono> 



int main()
{
    auto start = std::chrono::high_resolution_clock::now(); // Start the timer

    //manualTestingHash();

    std::unordered_set<std::pair<std::string, std::string>, CompareFirst, EqualFirst> urls;

    urls.insert(std::make_pair("http://en.wikipedia.org/wiki/Main_Page/", "")); //Initial link must be provided in this format

    int num_urls =  4;
    int max_wait = 30 * 1000; /* Wait max. 30 seconds */
    StripedHashSet<Webpage> HashSet(10);

    crawl_webpage(urls, num_urls, max_wait, HashSet);
    //TestingLikeTutorial();

    HashSet.print_all();


    int number = HashSet.get_count();
    std::cout << number << std::endl;

    auto end = std::chrono::high_resolution_clock::now(); // Stop the timer

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    
}