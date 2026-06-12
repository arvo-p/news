#ifndef FEED_HPP
#define FEED_HPP

#include <string>
#include <vector>
#include <fstream>
#include "globals.hpp"
#include "rapidxml-1.13/rapidxml.hpp"

struct Entry{
	std::string title;
	std::string link;
    rss_url rssUrl;
	struct tm date;
};

class feed{
	public:
		std::ofstream newsOutputStream;
		std::ifstream updateInputStream;

		feed();
		int Fetch(rss_url &rssUrl);
		int Parse(std::string &buffer, rss_url &rssUrl);
		int RecordEntry(std::string &entry_title, std::string &entry_link, rss_url &rssUrl, std::string &date_str);
		bool VerifyEntryDate(std::string &url_src, std::string &date);
		int ParseAtom(rss_url &rssUrl, rapidxml::xml_node<> *node);
		int ParseRdf(rss_url &rssUrl, rapidxml::xml_node<> *node);
		int ParseRss(rss_url &rssUrl, rapidxml::xml_node<> *node);
		bool CompareToLastUpdate(std::string &urlSrc, struct tm * param_t);
		bool UpdateRecord(std::string &urlSrc, struct tm * t);
		int SaveEntry(Entry &entry);
		int Save();
		int Close();

		std::vector<Entry> entryList;
		int parsedEntriesCount = 0;
};

#endif
