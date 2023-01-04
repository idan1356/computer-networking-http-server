#include "string_utils.h"

// Split a string by a delimiter
std::vector<std::string> splitString(const std::string& str, const char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> parts;

    size_t start = 0;
    size_t end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        parts.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }

    parts.push_back(str.substr(start));

    return parts;
}


std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \r\n");
    size_t last = str.find_last_not_of(" \r\n");
    return str.substr(first, (last - first + 1));
}
