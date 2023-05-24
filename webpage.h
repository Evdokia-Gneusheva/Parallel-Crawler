#include <iostream>
#ifndef WEBPAGE_H
#define WEBPAGE_H

struct Webpage
{
    std::string url;
    std::string content;
    std::vector<std::string> externalLinks;
};

#endif