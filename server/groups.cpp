#include "groups.hpp"
#include "files_path.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "functions.hpp"
#include <fstream>
#include <iostream>

using namespace std;

int groups::register_group(vector<string> &args, int lineNum){
	if(args.size() < 3){
		cout << "At " << lineNum << ": 3 arguments expected for register-group."  << endl;
		return 1;
	}

	if(!is_number(args[1])){
		cout << "At " << lineNum << ": Number expected for first argument." << endl;
		return 1;
	}

	custom_group new_group;
	new_group.name = args[2];
	new_group.id = std::stoi(args[1]);
	
	custom_groups.push_back(new_group);

	return 0;
}

groups::groups(){
	ifstream f;
	f.open(path->groups);
	if(f.is_open()){
		loaded = true;
		string line;
		vector <string> args;
		
		bool inGroups = false;

		int i=0;
		while(getline(f, line)){
			i++;
			if(inGroups){
				if((args = returnArgs(line)).empty()) continue; 
				if(args[0] == "register-group"){
					register_group(args,i-1);
				}
			}
			if(line == "@groups") inGroups = true; 
		}
	}
}
