#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <curl/curl.h>
#include <time.h>

#include "rapidxml-1.13/rapidxml.hpp"
#include "functions.hpp"

#include <shlobj.h>
#include <iomanip>

using namespace std;

vector<string> returnArgs(string line);

class files_path{
	public:
		string root = "kNews";
		string entries = "entries.dat";
		string update_record = "updates.dat";
		string feedlist = "webfeeds.conf";
		string blacklist = "blacklisted.conf";
		string groups = "user_groups.conf";

		string win_getHomeDir(){
			char pth_Appdata[MAX_PATH];
			SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pth_Appdata);
			string retstr(pth_Appdata);
			return retstr;			
		}

		int addPath(string &dest, string &tAdd){
			int len = dest.length();
			if(dest[len-1] != '\\') dest = dest + "\\";
			dest = dest + tAdd;

			return 0;
		}

		int addRootToPath(string &dest, string &tAdd){
			string ret = tAdd;
			addPath(ret, dest);
			dest = ret;

			return 0;
		}

		void add_root2filepaths(){
			string appdata = win_getHomeDir();

			addRootToPath(root, appdata);
			addRootToPath(entries, root);
			addRootToPath(feedlist, root);
			addRootToPath(blacklist, root);
			addRootToPath(groups, root);
			addRootToPath(update_record, root);
		}

		files_path(){
			add_root2filepaths();
			//checkIfFilesExistAndCreateIfNot();
		}
};

files_path * path;

class blacklist{
	public:
		vector<string> * words;
		vector<string> * urls;

		int load();
		bool check(string &title, string &url);
		
		blacklist(){
			load();
		}
};

struct rss_url{
	string url;
	vector <int> groups;
};

class feed{
	public:
		ofstream fNews;
		ifstream updates;

		int fetch(rss_url &rssUrl);
		int parse(string &buffer, rss_url &rssUrl);
		int record_entry(string &entry_title, string &entry_link, rss_url &rssUrl);
		bool VerifyEntryDate(string &url_src, string &date);
		int atomParse(rss_url &rssUrl, rapidxml::xml_node<> *node);
		int rssParse(rss_url &rssUrl, rapidxml::xml_node<> *node);
		bool CompareToLastUpdate(string &urlSrc, struct tm * param_t);
		bool Update_UpdateRecord(string &urlSrc, struct tm * t);

		int n_EntriesParsed = 0;

		feed(){
			fNews.open(path->entries, std::ios_base::app);
			if(!fNews.is_open()){
				cout << "Could not open " << path->entries << endl;
				exit(1);
			}
		}

		int close(){
			cout << "Cycle finished." << endl;
			fNews.close();
			return 0;
		}
};

struct custom_group{
	string name;
	int id;
};

class groups{
	public:
		bool loaded = false;

		vector<custom_group> custom_groups; 
		
		int register_group(vector<string> &args, int lineNum);

		groups(){
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
};


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

feed * mainRss;
blacklist * mainBlacklist;
groups * mainGroups;

int main(){
	path = new files_path();
	mainBlacklist = new blacklist();
	mainGroups = new groups();
	mainRss = new feed();
	
	ifstream f (path->feedlist);
	if(!f.is_open()){
		cout << path->feedlist << " was not found." << endl;
		return 1;
	}

	string line;
	vector <string> args, groups;

	while(getline(f, line)){
		args = returnArgs(line);
		rss_url newFeed;
		if(mainGroups->loaded){
			groups = returnArgs(args[1]);
			for(int i=0;i<groups.size();i++){
				for(custom_group registered: mainGroups->custom_groups){
					if(is_number(groups[i])){
						if(registered.id == std::stoi(groups[i])){
							newFeed.groups.push_back(registered.id);
							break;
						}
					}else{
						if(registered.name == groups[i]){
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

int main_cycle(){
	return 0;
}

vector<string> returnArgs(string line){
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

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

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

int append_to_number_number(int n, int a){
	int res = n / 10;
	int exp = 10;
	
	while(res>0){
		res = res / 10;
		exp *= 10;
	}

	return ((exp*a)+n);
}

unsigned long gen_new_ID(){
	unsigned long newID = mainRss->n_EntriesParsed;

	struct tm datetime;
	time_t now = time(NULL);
	srand(now);
	datetime = *localtime(&now);

	newID = append_to_number_number(rand()%9999, newID);	
	newID = append_to_number_number((datetime.tm_hour*60+datetime.tm_min)/5, newID);
	//mainRss->n_EntriesParsed;	
	return newID;
}

int feed::record_entry(string &entry_title, string &entry_link, rss_url &rssUrl){
	uint8_t unit_separator = 31;
	uint8_t sub_unit_separator = 30;

	fNews << std::hex << gen_new_ID() << unit_separator << entry_title << unit_separator << entry_link;
	
	int sz = rssUrl.groups.size();
	for(int i=0;i<sz;i++){
		if(i == 0){
			fNews << unit_separator;
		}else{
			fNews << " ";
		}
		fNews << rssUrl.groups[i];
	}

	//cout << "\nDebug: registering" << entry_link;
	//cout << "\nTitle: " << entry_title;
	//cout << "\nLink: " << entry_link;
	//
	fNews << endl;
	
	return 0;
}

bool blacklist::check(string &title, string &url){
	for(string &black:*(blacklist::words))
		if(title.find(black) != string::npos) return true;		
	for(string &black:*(blacklist::urls))
		if(url.find(black) != string::npos) return true;		
	return false;
}

bool getDatetime(string &date, string type, struct tm * t){
	
	std::istringstream ss(date);
	///memset(t, 0, sizeof(struct tm));

	int offset = -1;
	string timezone_string;
	int timezone_op;
	
	if(type == "RFC_1123"){	
		if(ss >> std::get_time(t, "%a, %d %b %Y %H:%M:%S")){
			offset = date.find_last_of(" ");
			if(offset == -1) return true;
		}
	}
	else if(type == "ISO_INSTANT"){
		if(ss >> std::get_time(t, "%Y-%m-%dT%H:%M:%S")){
			return true;
		}
	}

	if(offset != -1){
		timezone_string = date.substr(offset+1, 3);
		if(timezone_string == "GMT") return true;
		timezone_op = stoi(timezone_string, 0, 10); 
		t->tm_hour += timezone_op;
		t->tm_hour %= 24;
		return true;
	}
	
	return false;
}


bool feed::Update_UpdateRecord(string &urlSrc, struct tm * t){
	ofstream fw;
	ifstream fr;
	ostringstream filetext;
	string line;
	
	char output_date[20];
	uint8_t unit_separator = 31;

	fr.open(path->update_record);
	strftime(output_date, 20, "%Y-%m-%dT%H:%M:%S", t);

	filetext << urlSrc << unit_separator << output_date << endl;
	if(fr.is_open()){
		while(getline(fr, line)){	
			if(line.find(urlSrc) == -1) filetext << line << endl;
		}		
	}

	fr.close();

	fw.open(path->update_record);
	fw << filetext.str();
	fw.close();

	return true;
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

bool feed::CompareToLastUpdate(string &urlSrc, struct tm * param_t){
	
	ifstream f (path->update_record);
	string line;
	string saved_date_str;
	struct tm saved_date;

	bool found = false;

	if(f.is_open()){
		while(getline(f, line)){
			int offset = line.find(urlSrc);
			if(offset!=-1){
				saved_date_str = line.substr(urlSrc.length()+1);
				found = true;
			}
		}
	}

	if(!found) return true;
	if(!getDatetime(saved_date_str, "ISO_INSTANT", &saved_date)) return false;
	if(CompareDates(param_t, &saved_date) == 1) return true;

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

bool feed::VerifyEntryDate(string &url_src, string &date){
	/*
	UTC time of computer
	--------------------
	struct tm * utc_now;
	time_t mytime;
	time(&mytime);
	utc_now = gmtime(&mytime);
	*/
	
	struct tm dt_entry;

	int ret = getDatetime_all(date, &dt_entry);
	if(ret == 1){
		cout << "Failed to parse a date." << endl;
		return false;
	}

	
	return feed::CompareToLastUpdate(url_src, &dt_entry); 
}

int feed::parse(string &buffer, rss_url &rssUrl){
	int ret = 0;
	
	char * parsed = new char[buffer.length()+1];
	strcpy(parsed, buffer.c_str());

	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> *node;

	doc.parse<0>(parsed);

	cout << "* [" << rssUrl.url << "] " << endl;
	cout << "\tAttempting to fetch..." << endl;
	node = doc.first_node("rss");
	if(node == NULL){ //ATOM
		node = doc.first_node("feed");
		ret = feed::atomParse(rssUrl, node);
	}else ret = feed::rssParse(rssUrl, node);

	cout << "\tDONE" << endl;

	delete parsed;

	return ret;
}

int feed::rssParse(rss_url &rssUrl, rapidxml::xml_node<> *node){
	rapidxml::xml_node<> *item, *title_node, *link_node, *pdate_node;
	
	string pubdate_str;

	node = node->first_node("channel");
	
	pdate_node = node->first_node("lastBuildDate");
	if(pdate_node == NULL) pdate_node = node->first_node("pubDate");
	if(pdate_node == NULL) pdate_node = node->first_node("published");
	
	item = node->first_node("item");

	pubdate_str = pdate_node->value();
	if(feed::VerifyEntryDate(rssUrl.url, pubdate_str) == false){
		cout << "\tFeed already up to date" << endl;
		return 1;
	}

	struct tm t;
	getDatetime_all(pubdate_str, &t);
	
	while(item){
		string link_str, title_str, pubdate_str;

		link_node = item->first_node("link");
		title_node = item->first_node("title");
		
		pdate_node = item->first_node("pubDate");
	
		link_str = link_node->value();
		title_str = title_node->value();
		pubdate_str = pdate_node->value();
	
		if(title_str.size()==0){
			title_node = title_node->first_node();
			title_str = title_node->value();
		}
		
		bool blacklisted = mainBlacklist->check(title_str,link_str); 
		bool isEntryNew = feed::VerifyEntryDate(rssUrl.url,pubdate_str);

		if(blacklisted){
			//cout << "An entry was blacklisted." << endl;
		}else if(isEntryNew==true){
			mainRss->record_entry(title_str, link_str, rssUrl);
			mainRss->n_EntriesParsed++;
		}
		
		item = item->next_sibling("item");
	}

	feed::Update_UpdateRecord(rssUrl.url, &t);
	
	return 0;
}

int feed::atomParse(rss_url &rssUrl, rapidxml::xml_node<> *node){
	rapidxml::xml_node<> *item, *title_node, *link_node, *pdate_node;
	string pubdate_str;

	pdate_node = node->first_node("updated");
	if(pdate_node == NULL) pdate_node = node->first_node("published");

	item = node->first_node("entry");

	pubdate_str = pdate_node->value();
	if(feed::VerifyEntryDate(rssUrl.url, pubdate_str) == false){
		cout << "\tFeed already up to date" << endl;
		return 1;
	}

	struct tm t;
	getDatetime_all(pubdate_str, &t);

	while(item){
		string title_str, link_str, pubdate_str;

		title_node = item->first_node("title");
		link_node = item->first_node("link");
		pdate_node = item->first_node("published");
	
		title_str = title_node->value();
		link_str = link_node->first_attribute("href")->value();
		pubdate_str = pdate_node->value();
		
		bool blacklisted = mainBlacklist->check(title_str,link_str); 
		bool isEntryNew = feed::VerifyEntryDate(rssUrl.url,pubdate_str);

		if(blacklisted){
			//cout << "An entry was blacklisted." << endl;
		}else if(isEntryNew==true){
			mainRss->record_entry(title_str, link_str, rssUrl);
			mainRss->n_EntriesParsed++;
		}

		item = item->next_sibling("entry");
	}

	feed::Update_UpdateRecord(rssUrl.url, &t);

	return 0;
}

int feed::fetch(rss_url &rssUrl){
	CURL * curl;
	CURLcode res;

	//cout << "Debug: Fetching " << rssUrl.url;

	string &url = rssUrl.url;

	//cout << url;

	curl = curl_easy_init();
	if(!curl) return 1;

	string readBuffer;
	
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
   	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
    	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	
	res = curl_easy_perform(curl);
 	if(res != CURLE_OK){
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return 1;
	}

	mainRss->parse(readBuffer, rssUrl);

    curl_easy_cleanup(curl);
	
	return 0;
}
