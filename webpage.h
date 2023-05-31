#include <iostream>
#ifndef WEBPAGE_H
#define WEBPAGE_H

struct Webpage
{
    std::string url;
    char* content;
    std::vector<std::string> externalLinks;

    Webpage(const std::string& webpageUrl, const char* webpageContent, size_t contentSize)
        : url(webpageUrl), externalLinks()
    {
        content = (char*)malloc(contentSize);
        std::memcpy(content, webpageContent, contentSize);
    }
};

#endif