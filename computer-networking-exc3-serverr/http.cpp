#include "http.h"

HTTPRequest::HTTPRequest(string str) {
    originalRequest = str;
	vector<string> rows = splitString(str, "\r\n");
	vector<string> firstRow = splitString(rows[0], ' ');
    int i = 0;

    // extract method, query params and context path
	method = stringToMethod[firstRow[0]];
	queryParams = extractQueryParams(firstRow[1]);
	contextPath = splitString(firstRow[1], '?')[0];
	contextPath.erase(0, 1);

    // split each header line into key value and put in map
    while(++i < rows.size() && rows[i] != "") {
        vector<string> keyValueHeaderPair = splitString(rows[i], ':');
        headers[keyValueHeaderPair[0]] = trim(keyValueHeaderPair[1]);
    }

    // add body
    for (auto it = begin(rows) + i; it != end(rows); ++it) {
        body += *it;
    }

}

std::map<std::string, std::string> HTTPRequest::extractQueryParams(const std::string& url) {
    std::map<std::string, std::string> params;

    // find the start of the query string
    size_t queryStart = url.find('?');
    if (queryStart == std::string::npos) {
        return params;  // no query string found
    }

    // split the query string into key-value pairs
    std::string queryString = url.substr(queryStart + 1);
    std::stringstream ss(queryString);
    std::string keyValuePair;
    while (std::getline(ss, keyValuePair, '&')) {
        // split the key-value pair into a key and a value
        size_t equalsPos = keyValuePair.find('=');
        if (equalsPos == std::string::npos) {
            // no value specified, set it to an empty string
            params[keyValuePair] = "";
        }
        else {
            std::string key = keyValuePair.substr(0, equalsPos);
            std::string value = keyValuePair.substr(equalsPos + 1);
            params[key] = value;
        }
    }

    return params;
}
