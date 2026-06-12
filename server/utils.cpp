#include "utils.hpp"
#include "globals.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

vector<string> ParseArguments(string line){
	vector <string> args;
	string arg;
	short arg_index = 0;
	
	char comment_prefix = '#';
	char arg_separator = ' ';

	if(line.at(0) == comment_prefix) return args;

	if(line.at(0) == '[' && line.at(line.size()-1) == ']'){
		arg_separator = ',';
		line.erase(0,1);
		line.erase(line.size()-1, 1);
	}

	bool inBrackets = false;
	for(int i=0;i<line.size();i++){
		if(line.at(i) == '[') inBrackets = true;
		if(inBrackets && line.at(i) == ']') inBrackets = false;

		if(line.at(i) != arg_separator){
			arg += line[i];
		}

		if(!inBrackets && (line.at(i) == arg_separator || i+1 == line.size())){
			
			if(arg_separator != ' ' || arg_index != 0){
				short len = arg.length();
				for(short i=0;i<len;i++) arg[i] = tolower(arg[i]);
			}

			args.push_back(arg);
			arg_index++;
			arg = "";
		}
	}

	return args;
}

size_t CurlWriteData(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int ConcatenateNumbers(int n, int a){
	int res = n / 10;
	int exp = 10;
	
	while(res>0){
		res = res / 10;
		exp *= 10;
	}

	return ((exp*a)+n);
}

// GenerateNewId depends on rssManager, which is a global
#include "feed.hpp"
unsigned long GenerateNewId(){
	unsigned long newID = rssManager->parsedEntriesCount;

	struct tm datetime;
	time_t now = time(NULL);
	srand(now);
	datetime = *localtime(&now);

	newID = ConcatenateNumbers(rand()%9999, newID);	
	newID = ConcatenateNumbers((datetime.tm_hour*60+datetime.tm_min)/5, newID);
	return newID;
}

bool GetDatetime(string &date, string type, struct tm * t){
	int offset = -1;
	string timezone_string;
	int timezone_op;

	t->tm_isdst = 0;
	
	if(type == "RFC_1123"){	
		std::istringstream ss(date);
		if(ss >> std::get_time(t, "%a, %d %b %Y %H:%M:%S")){
			offset = date.find_last_of(" ");
		}
	}
	else if(type == "ISO_INSTANT"){
		size_t pos = date.find_first_of("+-Z", 10);
		std::istringstream ss(date.substr(0,pos));
		if(ss >> std::get_time(t, "%Y-%m-%dT%H:%M:%S")){
			if(pos != string::npos)offset = pos;
			else return true;
		}
	}

	if(offset != -1){
		if (date[offset] == 'Z') return true;
		timezone_string = date.substr(offset+1, 2);
		if(timezone_string == "GMT") return true;
		try {
			timezone_op = stoi(timezone_string, 0, 10); 
			if (date[offset] == '-') timezone_op = -timezone_op;
			t->tm_hour += timezone_op;
			mktime(t);
		} catch (...) {}
		return true;
	}
	
	return false;
}

void DebugPrintDate(struct tm * t){
	char buffer[64];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", t);
	std::cout << "	DEBUG_PRINTDATE " << buffer << std::endl; 
}

int ParseAnyDatetime(string &date, struct tm * t){
	if(!GetDatetime(date, "RFC_1123", t)){
		if(!GetDatetime(date, "ISO_INSTANT", t)){
			return 1;
		}
	}
	return 0;
}

int CompareDates(struct tm * d1, struct tm * d2){
	if(d1->tm_year>d2->tm_year) return 1;
	if(d1->tm_year<d2->tm_year) return 2;
	if(d1->tm_mon>d2->tm_mon) return 1;
	if(d1->tm_mon<d2->tm_mon) return 2;
	if(d1->tm_mday>d2->tm_mday) return 1;
	if(d1->tm_mday<d2->tm_mday) return 2;
	if(d1->tm_hour>d2->tm_hour) return 1;
	if(d1->tm_hour<d2->tm_hour) return 2;
	if(d1->tm_min>d2->tm_min) return 1;
	if(d1->tm_min<d2->tm_min) return 2;
	return 0;
}
