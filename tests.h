
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <list>
#include <iostream>
#include "webpage.h"
#include <algorithm>
#include <thread>
#include "hash_table.h"

void manualTestingHash()
{
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "Running the very simple test (no concurrency):" << std::endl;
    StripedHashSet<Webpage> myLittleHastable = StripedHashSet<Webpage>(100);

    Webpage myWebpage1;
    myWebpage1.url = "https://example.com";
    myWebpage1.content = "<html>...</html>";
    myWebpage1.externalLinks.push_back("https://external1.com");
    myWebpage1.externalLinks.push_back("https://external2.com");

    Webpage myWebpage2;
    myWebpage2.url = "https://example2.com";
    myWebpage2.content = "<html>...</html>";
    myWebpage2.externalLinks.push_back("https://external1.com");
    myWebpage2.externalLinks.push_back("https://external2.com");

    Webpage myWebpage3;
    myWebpage3.url = "https://example3.com";
    myWebpage3.content = "<html>...</html>";
    myWebpage3.externalLinks.push_back("https://external1.com");
    myWebpage3.externalLinks.push_back("https://external2.com");

    Webpage myWebpage4;
    myWebpage4.url = "https://example.com";
    myWebpage4.content = "<html>...</html>";
    myWebpage4.externalLinks.push_back("https://external1.com");
    myWebpage4.externalLinks.push_back("https://external2.com");

    std::cout << "Adding Webpage1..." << std::endl;
    bool result = myLittleHastable.add(myWebpage1);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Adding Webpage2..." << std::endl;
    result = myLittleHastable.add(myWebpage2);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Adding Webpage4..." << std::endl;
    result = myLittleHastable.add(myWebpage4);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Checking if contains Webpage4..." << std::endl;
    result = myLittleHastable.contains(myWebpage4);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Checking if contains Webpage1..." << std::endl;
    result = myLittleHastable.contains(myWebpage1);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Checking if contains Webpage3..." << std::endl;
    result = myLittleHastable.contains(myWebpage3);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Calling resize..." << std::endl;
    myLittleHastable.resize();

    std::cout << "Checking if contains Webpage4..." << std::endl;
    result = myLittleHastable.contains(myWebpage4);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Checking if contains Webpage1..." << std::endl;
    result = myLittleHastable.contains(myWebpage1);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Checking if contains Webpage3..." << std::endl;
    result = myLittleHastable.contains(myWebpage3);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Adding Webpage3..." << std::endl;
    result = myLittleHastable.add(myWebpage3);
    std::cout << "Result: " << result << std::endl;

    std::cout << "Checking if contains Webpage3..." << std::endl;
    result = myLittleHastable.contains(myWebpage3);
    std::cout << "Result: " << result << std::endl;
}

void simple_inserter(StripedHashSet<Webpage> &s, Webpage &str)
{
    s.add(str);
}

void TestingLikeTutorial()
{
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "Running the 'like tutorial' test (with threads):" << std::endl;

    StripedHashSet<Webpage> myLittleHastable = StripedHashSet<Webpage>(2);

    Webpage myWebpage1;
    myWebpage1.url = "https://example.com";
    myWebpage1.content = "<html>...</html>";
    myWebpage1.externalLinks.push_back("https://external1.com");
    myWebpage1.externalLinks.push_back("https://external2.com");

    Webpage myWebpage2;
    myWebpage2.url = "https://example2.com";
    myWebpage2.content = "<html>...</html>";
    myWebpage2.externalLinks.push_back("https://external1.com");
    myWebpage2.externalLinks.push_back("https://external2.com");

    Webpage myWebpage3;
    myWebpage3.url = "https://example3.com";
    myWebpage3.content = "<html>...</html>";
    myWebpage3.externalLinks.push_back("https://external1.com");
    myWebpage3.externalLinks.push_back("https://external2.com");

    Webpage myWebpage4;
    myWebpage4.url = "https://example4.com";
    myWebpage4.content = "<html>...</html>";
    myWebpage4.externalLinks.push_back("https://external1.com");
    myWebpage4.externalLinks.push_back("https://external2.com");

    std::vector<Webpage> items{myWebpage1, myWebpage2, myWebpage3, myWebpage4};
    std::vector<std::thread> inserters;

    for (auto it = items.begin(); it != items.end(); ++it)
    {
        inserters.emplace_back(std::thread(&simple_inserter, std::ref(myLittleHastable), std::ref(*it)));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    std::vector<size_t> sizes;

    while (myLittleHastable.get_count() > 0)
    {
        sizes.push_back(myLittleHastable.get_count());
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            if (myLittleHastable.contains(*it))
            {
                myLittleHastable.remove(*it);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                break;
            }
        }
    }

    std::for_each(inserters.begin(), inserters.end(), [](std::thread &t)
                  { t.join(); });

    std::cout << "Here are the sizes obtained: ";
    for (long long unsigned int i = 0; i < sizes.size(); i++)
    {

        std::cout << sizes[i] << " ";
    }

    std::cout << std::endl;
}
