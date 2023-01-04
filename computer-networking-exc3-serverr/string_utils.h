#pragma once
#include <sstream>
#include <vector>
#include <string>
#include <map>

std::vector<std::string> splitString(const std::string& str, const char delimiter);
std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);
std::string trim(const std::string& str);
