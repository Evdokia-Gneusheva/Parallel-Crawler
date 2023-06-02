#include <iostream>
#include <set>

#ifndef WEBPAGE_H
#define WEBPAGE_H

struct Webpage
{
    std::string url;
    std::set<std::string> externalLinks;
    int current_level;

    Webpage(const std::string &webpageUrl, std::set<std::string> links, int level)
        : url(webpageUrl), externalLinks(links), current_level(level) {}
};

#endif