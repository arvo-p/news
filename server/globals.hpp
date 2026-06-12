#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <string>
#include <vector>

struct rss_url {
    std::string url;
    std::vector<int> groups;
};

class files_path;
class feed;
class blacklist;
class groups;

extern files_path* pathManager;
extern feed* rssManager;
extern blacklist* blacklistManager;
extern groups* groupManager;

#endif
