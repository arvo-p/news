#include "groups.hpp"
#include "files_path.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "functions.hpp"
#include <fstream>
#include <iostream>

using namespace std;

int groups::RegisterGroup(vector<string> &args, int lineNum){
	if(args.size() < 3){
		cout << "At " << lineNum << ": 3 arguments expected for register-group."  << endl;
		return 1;
	}

	if(!IsNumber(args[1])){
		cout << "At " << lineNum << ": Number expected for first argument." << endl;
		return 1;
	}

	custom_group new_group;
	new_group.name = args[2];
	new_group.id = std::stoi(args[1]);
	
	groupList.push_back(new_group);

	return 0;
}

groups::groups(){
	ifstream f;
	f.open(pathManager->groupsFilePath);
	if(f.is_open()){
		isLoaded = true;
		string line;
		vector <string> args;
		
		bool inGroups = false;

		int i=0;
		while(getline(f, line)){
			i++;
			if(inGroups){
				if((args = ParseArguments(line)).empty()) continue; 
				if(args[0] == "register-group"){
					RegisterGroup(args,i-1);
				}
			}
			if(line == "@groups") inGroups = true; 
		}
	}
}
