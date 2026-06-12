#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

std::vector<std::string> ParseArguments(std::string line);
size_t CurlWriteData(void *contents, size_t size, size_t nmemb, void *userp);
int ConcatenateNumbers(int n, int a);
unsigned long GenerateNewId();
bool GetDatetime(std::string &date, std::string type, struct tm * t);
int ParseAnyDatetime(std::string &date, struct tm * t);
int CompareDates(struct tm * d1, struct tm * d2);
void DebugPrintDate(struct tm * t);

#endif
