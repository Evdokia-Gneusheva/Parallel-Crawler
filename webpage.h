#include <iostream>
#include <unordered_set>

#ifndef WEBPAGE_H
#define WEBPAGE_H

struct Webpage
{
    std::string parent;
    std::string url;
    std::unordered_set<std::string> externalLinks;
    int current_level;


    Webpage() : parent(), url(), externalLinks(), current_level(0) {}  // Empty constructor


    Webpage(const std::string &webpageUrl, std::unordered_set<std::string> links, int level, std::string parentUrl)
        : url(webpageUrl),  parent(parentUrl), externalLinks(links), current_level(level) {}
};

#endif