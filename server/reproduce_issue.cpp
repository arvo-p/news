#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace std;

bool getDatetime(string &date, string type, struct tm * t){

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
			cout << "Timezone string: " << timezone_string << endl;
			if (date[offset] == '-') timezone_op = -timezone_op;
			t->tm_hour += timezone_op;
			mktime(t);
		} catch (...) {}
		return true;
	}
	
	return false;
}

int getDatetime_all(string &date, struct tm * t){
	if(!getDatetime(date, "RFC_1123", t)){
		if(!getDatetime(date, "ISO_INSTANT", t)){
			return 1;
		}
	}
	return 0;
}

int main() {
    string date = "2016-10-15T10:16:29+00:00";
    struct tm t = {0};
    int ret = getDatetime_all(date, &t);
    if (ret == 0) {
        char buf[100];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
		cout << "Source: " << date << endl;
        cout << "Parsed successfully: " << buf << endl;
    } else {
        cout << "Failed to parse." << endl;
    }


    return 0;
}
