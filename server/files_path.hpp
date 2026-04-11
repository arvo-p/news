#ifndef FILES_PATH_HPP
#define FILES_PATH_HPP

#include <string>
#include <windows.h>
#include <shlobj.h>

class files_path{
	public:
		std::string root = "kNews";
		std::string entries = "entries.dat";
		std::string update_record = "updates.dat";
		std::string feedlist = "webfeeds.conf";
		std::string blacklist = "blacklisted.conf";
		std::string groups = "user_groups.conf";

		std::string win_getHomeDir();
		int addPath(std::string &dest, std::string &tAdd);
		int addRootToPath(std::string &dest, std::string &tAdd);
		void add_root2filepaths();

		files_path();
};

#endif
