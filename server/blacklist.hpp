#ifndef BLACKLIST_HPP
#define BLACKLIST_HPP

#include <vector>
#include <string>

class blacklist{
	public:
		std::vector<std::string> * words;
		std::vector<std::string> * urls;

		int load();
		bool check(std::string &title, std::string &url);
		
		blacklist();
};

#endif
