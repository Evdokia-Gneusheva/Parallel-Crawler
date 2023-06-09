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
#include <stdlib.h> //for using the function sleep
#include <fstream>

std::ofstream file("output.txt", std::ios::app); // Open the file "output.txt" for writing

void execute_process(bool only_local = true, int max_links_per_thread = 50, int max_level = 3, int disregard_leftovers = false, int numthreads = 2, int num_links = 1000)
{

    auto start = std::chrono::high_resolution_clock::now(); // Start the timer

    // manualTestingHash();

    std::unordered_set<std::pair<std::string, std::string>, CompareFirst, EqualFirst> urls;

    urls.insert(std::make_pair("http://en.wikipedia.org/wiki/Main_Page/", "")); // Initial link must be provided in this format

    int num_urls = 4;
    int max_wait = 30 * 1000; /* Wait max. 30 seconds */
    StripedHashSet<Webpage> HashSet(100);

    crawl_webpage(urls, num_urls, max_wait, HashSet, only_local, max_links_per_thread, max_level, disregard_leftovers, numthreads, num_links);
    // TestingLikeTutorial();

    // HashSet.print_all();

    int number = HashSet.get_count();

    auto end = std::chrono::high_resolution_clock::now(); // Stop the timer

    std::chrono::duration<double> elapsed = end - start;

    if (file.is_open())
    {
        std::cout << "--------------------------------------------" << std::endl;
        std::cout << "Elapsed time: " << elapsed.count() << " seconds, using " << numthreads << " threads" << std::endl;
        std::cout << "Links visited: " << number << ", Tried to visit:" << num_links << std::endl;
        std::cout << "Links per thread: " << max_links_per_thread << std::endl;
        std::cout << "--------------------------------------------" << std::endl;
        //std::cout << "Data written to the file." << std::endl;
    
    }
    else
    {
        //std::cout << "Failed to open the file." << std::endl;
    }
}

void test(){

 int thread_numbers[] = {8, 16, 32};
    int links_per_thread_numbers[] = {16, 8, 32};
    int links_number[] = {10000};

    int thread_numbers_size = sizeof(thread_numbers) / sizeof(thread_numbers[0]);
    int links_per_thread_size = sizeof(links_per_thread_numbers) / sizeof(links_per_thread_numbers[0]);
    int links_number_size = sizeof(links_number) / sizeof(links_number[0]);

    for (int j = 0; j < links_per_thread_size; ++j)
    {
        for (int i = 0; i < thread_numbers_size; ++i)
        {
            for (int k = 0; k < links_number_size; ++k)
            {
                int num_threads = thread_numbers[i];
                int link_per_thread = links_per_thread_numbers[j];
                int num_links = links_number[k];

                for (int m = 0; m < 3; ++m)
                {
                    if ((i != 1 || k != 10000) && (i != 1 || k != 5000) && (i != 2 || k != 10000))
                    {

                        execute_process(true, link_per_thread, 1000, false, num_threads, num_links);
                        has_level_been_reached = false;
                        counter = 0;
                    }
                }
            }
        }
    }


}


int main(int argc, char *argv[]) {

    bool only_local = true;
    int max_links_per_thread = 32;
    int max_level = 1000;
    bool disregard_leftovers = false;
    int numthreads = 16;
    int num_links = 100;


    if (argc == 7) {
        only_local = std::atoi(argv[1]);
        max_links_per_thread = std::atoi(argv[2]);
        max_level = std::atoi(argv[3]);
        disregard_leftovers = std::atoi(argv[4]);
        numthreads = std::atoi(argv[5]);
        num_links = std::atoi(argv[6]);

        //std::cout << only_local << max_links_per_thread << max_level << disregard_leftovers << numthreads << num_links << std::endl;

    } else if (argc > 1) {
        std::cerr << "Usage: " << argv[0] << " [only_local] [max_links_per_thread] [max_level] [disregard_leftovers] [num_threads] [num_links]\n";
        return 1;
    }

    execute_process(only_local, max_links_per_thread, max_level, disregard_leftovers, numthreads, num_links);
    return 0;
}