#ifndef FEED_HPP
#define FEED_HPP

#include <string>
#include <vector>
#include <fstream>
#include "globals.hpp"
#include "rapidxml-1.13/rapidxml.hpp"

class feed{
	public:
		std::ofstream fNews;
		std::ifstream updates;

		int fetch(rss_url &rssUrl);
		int parse(std::string &buffer, rss_url &rssUrl);
		int record_entry(std::string &entry_title, std::string &entry_link, rss_url &rssUrl);
		bool VerifyEntryDate(std::string &url_src, std::string &date);
		int atomParse(rss_url &rssUrl, rapidxml::xml_node<> *node);
		int rssParse(rss_url &rssUrl, rapidxml::xml_node<> *node);
		bool CompareToLastUpdate(std::string &urlSrc, struct tm * param_t);
		bool Update_UpdateRecord(std::string &urlSrc, struct tm * t);

		int n_EntriesParsed = 0;

		feed();
		int close();
};

#endif
