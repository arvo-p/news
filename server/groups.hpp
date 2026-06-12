#ifndef GROUPS_HPP
#define GROUPS_HPP

#include <string>
#include <vector>

struct custom_group{
	std::string name;
	int id;
};

class groups{
	public:
		bool isLoaded = false;

		std::vector<custom_group> groupList; 
		
		int RegisterGroup(std::vector<std::string> &args, int lineNum);

		groups();
};

#endif
