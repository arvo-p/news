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
files_path * path;
feed * mainRss;
blacklist * mainBlacklist;
groups * mainGroups;

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
	mainRss->parse(content, testFeed);
}

int main(int argc, char** argv){
	path = new files_path();
	mainBlacklist = new blacklist();
	mainGroups = new groups();
	mainRss = new feed();
	
	if(argc > 1 && string(argv[1]) == "test"){
		string fileToTest = "example.atom";
		if(argc > 2) fileToTest = argv[2];
		testLocalFile(fileToTest);
		mainRss->close();
		return 0;
	}

	ifstream f (path->feedlist);
	if(!f.is_open()){
		cout << path->feedlist << " was not found." << endl;
		return 1;
	}

	string line;
	vector <string> args, groups_vec;

	while(getline(f, line)){
		args = returnArgs(line);
		if(args.empty()) continue;
		
		rss_url newFeed;
		if(mainGroups->loaded && args.size() > 1){
			groups_vec = returnArgs(args[1]);
			for(int i=0;i<groups_vec.size();i++){
				for(custom_group registered: mainGroups->custom_groups){
					if(is_number(groups_vec[i])){
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
		mainRss->fetch(newFeed);
	}

	mainRss->close();

	return 0;
}
