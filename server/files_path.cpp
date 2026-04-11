#include "files_path.hpp"

using namespace std;

string files_path::win_getHomeDir(){
	char pth_Appdata[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pth_Appdata);
	string retstr(pth_Appdata);
	return retstr;			
}

int files_path::addPath(string &dest, string &tAdd){
	int len = dest.length();
	if(dest[len-1] != '\\') dest = dest + "\\";
	dest = dest + tAdd;

	return 0;
}

int files_path::addRootToPath(string &dest, string &tAdd){
	string ret = tAdd;
	addPath(ret, dest);
	dest = ret;

	return 0;
}

void files_path::add_root2filepaths(){
	string appdata = win_getHomeDir();

	addRootToPath(root, appdata);
	addRootToPath(entries, root);
	addRootToPath(feedlist, root);
	addRootToPath(blacklist, root);
	addRootToPath(groups, root);
	addRootToPath(update_record, root);
}

files_path::files_path(){
	add_root2filepaths();
}
