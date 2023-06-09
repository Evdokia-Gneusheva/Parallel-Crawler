#include <vector>
#include <mutex>
#include <shared_mutex>
#include <list>
#include <iostream>
#include "webpage.h"

#ifndef HASHSET_H
#define HASHSET_H

bool operator==(const Webpage &lhs, const Webpage &rhs)
{
    if (lhs.url == rhs.url)
    {
        return true;
    }
    return false;
}

template <typename T>
class StripedHashSet
{
private:
    std::vector<std::mutex> locks;
    std::vector<std::list<T>> table;

public:
    StripedHashSet(int capacity)
        : locks(capacity), table(capacity) {}

    bool contains(T x)
    {
        std::lock_guard<std::mutex> lock(locks[std::hash<std::string>{}(x.url) % locks.size()]);
        int myBucket = std::hash<std::string>{}(x.url) % table.size();
        for (auto it = table[myBucket].begin(); it != table[myBucket].end(); ++it)
        {
            if (*it == x)
            {
                return true;
            }
        }
        return false;
    }

    bool add(T x)
    {   
        int myBucket = std::hash<std::string>{}(x.url) % table.size();
        {
            std::lock_guard<std::mutex> lock(locks[myBucket]);
            for (auto it = table[myBucket].begin(); it != table[myBucket].end(); ++it)
            {
                if (*it == x)
                {
                    return false;
                }
            }
            table[myBucket].push_back(x);
        }
        
        if (table[myBucket].size() > table.size())
        {   
            resize();
        }
        return true;
    }

    bool remove(T x)
    {
        std::lock_guard<std::mutex> lock(locks[std::hash<std::string>{}(x.url) % locks.size()]);
        int myBucket = std::hash<std::string>{}(x.url) % table.size();
        for (auto it = table[myBucket].begin(); it != table[myBucket].end(); ++it)
        {
            if (*it == x)
            {
                table[myBucket].erase(it);
                return true;
            }
        }
        return false;
    }

    bool add_links(T &x, const std::unordered_set<std::string> &links)
    {

        std::lock_guard<std::mutex> lock(locks[std::hash<std::string>{}(x.url) % locks.size()]);
        int myBucket = std::hash<std::string>{}(x.url) % table.size();
        for (auto it = table[myBucket].begin(); it != table[myBucket].end(); ++it)
        {

            if (*it == x)
            {
                it->externalLinks.insert(links.begin(), links.end());
                return true;
            }
        }
        return false;
    }

void resize()
{

    std::vector<std::list<T>> newTable(table.size() * 2);
    std::vector<std::mutex> newLocks(table.size() * 2); // Updated locks vector

    std::vector<std::unique_lock<std::mutex>> currentLocks;
    for (auto &lock : locks) {
        currentLocks.emplace_back(lock);
    }

    for (long long unsigned int i = 0; i < table.size(); i++)
    {
        for (auto it : table[i])
        {
            int myBucket = std::hash<std::string>{}(it.url) % newTable.size();
            newTable[myBucket].push_back(it);
        }
    }

    table = newTable;
    locks = std::move(newLocks); // Update the locks vector with the new locks
    std::cout << "Hello im ending resizing" << std::endl;

}

    std::list<T> get_bucket(T x)
    {
        return table[std::hash<std::string>{}(x.url) % table.size()];
    }

    int get_count()
    {
        int count = 0;
        for (long long unsigned int i = 0; i < locks.size(); i++)
        {
            std::lock_guard<std::mutex> lock(locks[i]);
        }

        for (auto &bucket : table)
        {
            count += bucket.size();
            // std::cout << "Number of items in a bucket: " << bucket.size() << std::endl;
        }

        return count;
    }

    Webpage *get(const std::string &url)
    {
        std::lock_guard<std::mutex> lock(locks[std::hash<std::string>{}(url) % locks.size()]);
        int myBucket = std::hash<std::string>{}(url) % table.size();
        for (auto it = table[myBucket].begin(); it != table[myBucket].end(); ++it)
        {
            if (it->url == url)
            {
                return &(*it);
            }
        }
        return nullptr;
    }

    void print_all()
    {
        for (long long unsigned int i = 0; i < locks.size(); i++)
        {
            std::lock_guard<std::mutex> lock(locks[i]);
        }

        for (long long unsigned int i = 0; i < table.size(); i++)
        {
            std::cout << "Bucket " << i << ":\n";
            for (auto &element : table[i])
            {
                std::cout << element.url << "\n";
            }
            std::cout << "\n";
        }
    }
};

#endif