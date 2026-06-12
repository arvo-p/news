#include "feed.hpp"
#include "files_path.hpp"
#include "blacklist.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>
#include <curl/curl.h>
#include <cstring>
#include <ctime>

using namespace std;

feed::feed(){
	newsOutputStream.open(pathManager->entriesFilePath, std::ios_base::app);
	if(!newsOutputStream.is_open()){
		cout << "Could not open " << pathManager->entriesFilePath << endl;
		exit(1);
	}
}

int feed::Save(){
	cout << "Sorting" << endl;
	std::sort(entryList.begin(), entryList.end(), [](const Entry& a, const Entry& b){
		struct tm tmA = a.date;
		struct tm tmB = b.date;
		return std::mktime(&tmA) < std::mktime(&tmB);
	});

	cout << "Saving entries" << endl;
	for(auto& entry: entryList)
		SaveEntry(entry);
	return 0;
}

int feed::Close(){
	cout << "Operation exited normally" << endl;
	entryList.clear();
	newsOutputStream.close();
	return 0;
}

int feed::SaveEntry(Entry &entry){
	uint8_t unit_separator = 31;
	newsOutputStream << std::hex << GenerateNewId() << unit_separator << entry.title << unit_separator << entry.link;
	
	int sz = entry.rssUrl.groups.size();
	for(int i=0;i<sz;i++){
		if(i == 0) newsOutputStream << unit_separator;
		else newsOutputStream << " ";
		newsOutputStream << entry.rssUrl.groups[i];
	}
	newsOutputStream << endl;
	return 0;
}

int feed::RecordEntry(string &entry_title, string &entry_link, rss_url &rssUrl, string &date_str){
	Entry entry;
	entry.title = entry_title;
	entry.link = entry_link;
	entry.rssUrl = rssUrl;
	
	struct tm date;
	ParseAnyDatetime(date_str, &date);
	entry.date = date;

	entryList.push_back(entry);
	return 0;
}

bool feed::UpdateRecord(string &urlSrc, struct tm * t){
	ofstream fw;
	ifstream fr;
	ostringstream filetext;
	string line;
	char output_date[20];
	uint8_t unit_separator = 31;

	fr.open(pathManager->updateRecordFilePath);
	strftime(output_date, 20, "%Y-%m-%dT%H:%M:%S", t);

	filetext << urlSrc << unit_separator << output_date << endl;
	if(fr.is_open()){
		while(getline(fr, line)){	
			if(line.find(urlSrc) == -1) filetext << line << endl;
		}		
	}
	fr.close();

	fw.open(pathManager->updateRecordFilePath);
	fw << filetext.str();
	fw.close();

	return true;
}

bool feed::CompareToLastUpdate(string &urlSrc, struct tm * param_t){
	ifstream f (pathManager->updateRecordFilePath);
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
	if(!GetDatetime(saved_date_str, "ISO_INSTANT", &saved_date)) return false;
	if(CompareDates(param_t, &saved_date) == 1) return true;

	return false;
}

bool feed::VerifyEntryDate(string &url_src, string &date){
	struct tm dt_entry;
	int ret = ParseAnyDatetime(date, &dt_entry);
	if(ret == 1){
		cout << "Failed to parse a date." << endl;
		return false;
	}
	return feed::CompareToLastUpdate(url_src, &dt_entry); 
}

int feed::Parse(string &buffer, rss_url &rssUrl){
	int ret = 0;
	char * parsed = new char[buffer.length()+1];
	strcpy(parsed, buffer.c_str());

	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> *node;
	
	try{
   		doc.parse<0>(parsed);
	}catch(const exception& e){
     	cout << "Failure to fetch :( " << rssUrl.url << endl;
		cout << "Trying next" << endl;
		return 1;
	}

	cout << "* [" << rssUrl.url << "] " << endl;
	cout << "\tFetching." << endl;
	
	node = doc.first_node("rss");
	if(node == NULL){
		node = doc.first_node("feed");
		if(node == NULL){
			node = doc.first_node("rdf:RDF");
			ret = feed::ParseRdf(rssUrl, node);
		}else ret = feed::ParseAtom(rssUrl, node);
	}else ret = feed::ParseRss(rssUrl, node);

	delete[] parsed;
	return ret;
}

int feed::ParseRss(rss_url &rssUrl, rapidxml::xml_node<> *node){
	cout << "\tParsing RSS" << endl;
	rapidxml::xml_node<> *item, *title_node, *link_node, *pdate_node;
	string pubdate_str;

	node = node->first_node("channel");
	pdate_node = node->first_node("published");
	pdate_node = node->first_node("lastBuildDate");
	if(pdate_node == NULL) pdate_node = node->first_node("pubDate");
	
	item = node->first_node("item");
	pubdate_str = pdate_node->value();
	if(feed::VerifyEntryDate(rssUrl.url, pubdate_str) == false){
		cout << "\tFeed already up to date" << endl;
		return 1;
	}

	struct tm t;
	ParseAnyDatetime(pubdate_str, &t);

	while(item){
		string link_str, title_str, pubdate_str;
		link_node = item->first_node("link");
		title_node = item->first_node("title");
		pdate_node = item->first_node("pubDate");
		if(!link_node || !title_node || !pdate_node) break;

		link_str = link_node->value();
		title_str = title_node->value();
		pubdate_str = pdate_node->value();

		if(title_str.size()==0){
			title_node = title_node->first_node();
			if(title_node) title_str = title_node->value();
			else title_str="NO TITLE";
		}
		
		bool blacklisted = blacklistManager->Check(title_str,link_str); 
		bool isEntryNew = feed::VerifyEntryDate(rssUrl.url,pubdate_str);

		if(!blacklisted && isEntryNew){
			rssManager->RecordEntry(title_str, link_str, rssUrl, pubdate_str);
			rssManager->parsedEntriesCount++;
		}
		item = item->next_sibling("item");
	}
	feed::UpdateRecord(rssUrl.url, &t);
	return 0;
}

int feed::ParseRdf(rss_url &rssUrl, rapidxml::xml_node<> *node){
	struct tm t, tmNew;
	cout << "\tParsing RDF" << endl;
	rapidxml::xml_node<> *item, *title_node, *link_node, *pdate_node;
	string pubdate_str;

	item = node->first_node("item");

	int i=0;
	while(item){
		string title_str, link_str, pubdate_str;
		title_node = item->first_node("title");
		link_node = item->first_node("link");

		pdate_node = item->first_node("dc:date");

		title_str = title_node->value();
		link_str = link_node->value();
		pubdate_str = pdate_node->value();
	
		ParseAnyDatetime(pubdate_str, &t);
		if(i++ == 0){
			tmNew = t;
		}else if(CompareDates(&t, &tmNew) == 1) tmNew = t;

		bool blacklisted = blacklistManager->Check(title_str,link_str); 
		bool isEntryNew = feed::VerifyEntryDate(rssUrl.url,pubdate_str);

		if(!blacklisted && isEntryNew){
			rssManager->RecordEntry(title_str, link_str, rssUrl, pubdate_str);
			rssManager->parsedEntriesCount++;
		}
		item = item->next_sibling("item");
	}

	feed::UpdateRecord(rssUrl.url, &tmNew);

	return 0;
}

int feed::ParseAtom(rss_url &rssUrl, rapidxml::xml_node<> *node){
	cout << "\tParsing ATOM" << endl;
	rapidxml::xml_node<> *item, *title_node, *link_node, *pdate_node, *headerdate_node;
	string pubdate_str;

	headerdate_node = node->first_node("updated");
	if(!headerdate_node) headerdate_node = node->first_node("lastBuildDate");

	struct tm headerDatetime;
	bool headerDefined = false;

	if(headerdate_node){
		headerDefined = true;
		pubdate_str = headerdate_node->value();
	    ParseAnyDatetime(pubdate_str, &headerDatetime); 
		if(feed::VerifyEntryDate(rssUrl.url, pubdate_str) == false){
			cout << "\tFeed already up to date" << endl;
			return 1;
		}
	}

	item = node->first_node("entry");
	while(item){
		string title_str, link_str, pubdate_str;
		title_node = item->first_node("title");
		link_node = item->first_node("link");

		pdate_node = item->first_node("updated");
		if(!pdate_node) pdate_node = item->first_node("published");

		title_str = title_node->value();
		link_str = link_node->first_attribute("href")->value();
		pubdate_str = pdate_node->value();

		struct tm itemDatetime;
		ParseAnyDatetime(pubdate_str,&itemDatetime);
		
		/* I noticed the YouTube header only has the <published> tag, which corresponds to the date of channel creation and
		 * does not actually tell us the last update time. So I need to get from the item by doing a comparison.*/

		if(!headerDefined || CompareDates(&headerDatetime, &itemDatetime) == 2){
			headerDatetime = itemDatetime;    
			headerDefined = true;
		}
	
		bool blacklisted = blacklistManager->Check(title_str,link_str); 
		bool isEntryNew = feed::VerifyEntryDate(rssUrl.url,pubdate_str);

		if(!blacklisted && isEntryNew){
			rssManager->RecordEntry(title_str, link_str, rssUrl, pubdate_str);
			rssManager->parsedEntriesCount++;
		}
		item = item->next_sibling("entry");
		//DebugPrintDate(&itemDatetime);
	}
	feed::UpdateRecord(rssUrl.url, &headerDatetime);
	//DebugPrintDate(&headerDatetime);
	return 0;
}


int feed::Fetch(rss_url &rssUrl){
	CURL * curl;
	CURLcode res;
	string &url = rssUrl.url;
	curl = curl_easy_init();
	if(!curl) return 1;

	string readBuffer;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
   	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
	curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteData);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	
	res = curl_easy_perform(curl);
 	if(res != CURLE_OK){
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	rssManager->Parse(readBuffer, rssUrl);
    curl_easy_cleanup(curl);
	return 0;
}
