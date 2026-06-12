#ifndef BLACKLIST_HPP
#define BLACKLIST_HPP

#include <vector>
#include <string>

class blacklist{
	public:
		std::vector<std::string> * blacklistedWords;
		std::vector<std::string> * blacklistedUrls;

		int Load();
		bool Check(std::string &title, std::string &url);
		
		blacklist();
};

#endif
