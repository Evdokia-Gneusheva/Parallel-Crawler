
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

