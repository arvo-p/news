#include "blacklist.hpp"
#include "files_path.hpp"
#include "globals.hpp"
#include <fstream>
#include <string>

using namespace std;

int blacklist::load(){
	ifstream f (path->blacklist);

	string line;

	vector<string> * vUrls = new vector<string>;
	vector<string> * vWords = new vector<string>;

	int appendTo = 0;

	if(f.is_open()){
		while(getline(f, line)){
			if(line == "@blacklisted_urls") appendTo = 1;
			if(line == "@blacklisted_words") appendTo = 0;
			if(line[0] == '@') continue;

			if(line.length() > 3){
				if(appendTo) (*vUrls).push_back(line);
				else (*vWords).push_back(line);
			}
		}
	}

	words = vWords;
	urls = vUrls;

	return 0;
}

bool blacklist::check(string &title, string &url){
	for(string &black:*(blacklist::words))
		if(title.find(black) != string::npos) return true;		
	for(string &black:*(blacklist::urls))
		if(url.find(black) != string::npos) return true;		
	return false;
}

blacklist::blacklist(){
	load();
}
