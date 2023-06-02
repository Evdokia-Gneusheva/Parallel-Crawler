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
        std::lock_guard<std::mutex> lock(locks[std::hash<std::string>{}(x.url) % locks.size()]);
        int myBucket = std::hash<std::string>{}(x.url) % table.size();
        for (auto it = table[myBucket].begin(); it != table[myBucket].end(); ++it)
        {
            if (*it == x)
            {
                return false;
            }
        }
        table[myBucket].push_back(x);
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

    bool add_links(T& x, const std::set<std::string>& links) {

        std::lock_guard<std::mutex> lock(locks[std::hash<std::string>{}(x.url) % locks.size()]);
        int myBucket = std::hash<std::string>{}(x.url) % table.size();
        for (auto it = table[myBucket].begin(); it != table[myBucket].end(); ++it) {

            if (*it == x) {
                it->externalLinks.insert(links.begin(), links.end());
                return true;
            }
        }
        return false;

    }


    void resize()
    {
        std::vector<std::list<T>> newTable(table.size() * 2);

        for (long long unsigned int i = 0; i < locks.size(); i++)
        {
            std::lock_guard<std::mutex> lock(locks[i]);
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
};

#endif