#ifndef FILES_PATH_HPP
#define FILES_PATH_HPP

#include <string>
#include <windows.h>
#include <shlobj.h>

class files_path{
	public:
		std::string rootDir = "kNews";
		std::string entriesFilePath = "entries.dat";
		std::string updateRecordFilePath = "updates.dat";
		std::string feedListFilePath = "webfeeds.conf";
		std::string blacklistFilePath = "blacklisted.conf";
		std::string groupsFilePath = "user_groups.conf";

		std::string GetHomeDirectory();
		int AddPath(std::string &dest, std::string &tAdd);
		int AddRootToPath(std::string &dest, std::string &tAdd);
		void ApplyRootToPaths();

		files_path();
};

#endif
