#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include "globals.hpp"
#include "files_path.hpp"
#include "blacklist.hpp"
#include "groups.hpp"
#include "feed.hpp"
#include "utils.hpp"
#include "functions.hpp"

using namespace std;

// Global pointers
files_path * pathManager;
feed * rssManager;
blacklist * blacklistManager;
groups * groupManager;

void testLocalFile(string filename){
	cout << "Testing with local file: " << filename << endl;
	ifstream t(filename);
	if(!t.is_open()){
		cout << "Could not open " << filename << endl;
		return;
	}
	stringstream buffer;
	buffer << t.rdbuf();
	string content = buffer.str();
	
	rss_url testFeed;
	testFeed.url = "local://" + filename;
	rssManager->Parse(content, testFeed);
	}

	int main(int argc, char** argv){
	pathManager = new files_path();
	blacklistManager = new blacklist();
	groupManager = new groups();
	rssManager = new feed();

	if(argc > 1 && string(argv[1]) == "test"){
		string fileToTest = "example.atom";
		if(argc > 2) fileToTest = argv[2];
		testLocalFile(fileToTest);
		rssManager->Close();
		return 0;
	}

	ifstream f (pathManager->feedListFilePath);
	if(!f.is_open()){
		cout << pathManager->feedListFilePath << " was not found." << endl;
		return 1;
	}

	string line;
	vector <string> args, groups_vec;
	while(getline(f, line)){
		args = ParseArguments(line);
		if(args.empty()) continue;

		rss_url newFeed;
		if(groupManager->isLoaded && args.size() > 1){
			groups_vec = ParseArguments(args[1]);
			for(int i=0;i<groups_vec.size();i++){
				for(custom_group registered: groupManager->groupList){
					if(IsNumber(groups_vec[i])){
						if(registered.id == std::stoi(groups_vec[i])){
							newFeed.groups.push_back(registered.id);
							break;
						}
					}else{
						if(registered.name == groups_vec[i]){
							newFeed.groups.push_back(registered.id);
							break;
						}
					}
				}
			}
		}

		newFeed.url = args[0];
		rssManager->Fetch(newFeed);
	}
	rssManager->Save();
	rssManager->Close();

	return 0;
	}
