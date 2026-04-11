#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

std::vector<std::string> returnArgs(std::string line);
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
int append_to_number_number(int n, int a);
unsigned long gen_new_ID();
bool getDatetime(std::string &date, std::string type, struct tm * t);
int getDatetime_all(std::string &date, struct tm * t);
int CompareDates(struct tm * d1, struct tm * d2);

#endif
