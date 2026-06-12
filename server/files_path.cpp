#include "files_path.hpp"

using namespace std;

string files_path::GetHomeDirectory(){
	char pth_Appdata[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pth_Appdata);
	string retstr(pth_Appdata);
	return retstr;			
}

int files_path::AddPath(string &dest, string &tAdd){
	int len = dest.length();
	if(dest[len-1] != '\\') dest = dest + "\\";
	dest = dest + tAdd;

	return 0;
}

int files_path::AddRootToPath(string &dest, string &tAdd){
	string ret = tAdd;
	AddPath(ret, dest);
	dest = ret;

	return 0;
}

void files_path::ApplyRootToPaths(){
	string appdata = GetHomeDirectory();

	AddRootToPath(rootDir, appdata);
	AddRootToPath(entriesFilePath, rootDir);
	AddRootToPath(feedListFilePath, rootDir);
	AddRootToPath(blacklistFilePath, rootDir);
	AddRootToPath(groupsFilePath, rootDir);
	AddRootToPath(updateRecordFilePath, rootDir);
}

files_path::files_path(){
	ApplyRootToPaths();
}
