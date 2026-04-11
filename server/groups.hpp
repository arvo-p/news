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
		bool loaded = false;

		std::vector<custom_group> custom_groups; 
		
		int register_group(std::vector<std::string> &args, int lineNum);

		groups();
};

#endif
