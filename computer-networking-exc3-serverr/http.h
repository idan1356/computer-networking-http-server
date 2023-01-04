#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <ctime>

#include "string_utils.h"
using namespace std;

class HTTPRequest {
public:
	enum HTTPMethod {GET_OR_HEAD, TRACE, PUT, HEAD, POST, OPTIONS, DELETE_};

private:
	HTTPMethod method;
	string contextPath;
	map<string, string> queryParams;
	map<string, string> headers;
	string body;

	string originalRequest;

	map<string, HTTPMethod> stringToMethod = {
		{"GET",HTTPMethod::GET_OR_HEAD}, {"PUT",HTTPMethod::PUT},
		{"TRACE",HTTPMethod::TRACE}, {"DELETE",HTTPMethod::DELETE_},
		{"HEAD",HTTPMethod::HEAD}, {"OPTIONS", HTTPMethod::OPTIONS},{"POST",HTTPMethod::POST}
	};

	std::map<std::string, std::string> extractQueryParams(const std::string& url);

public:
	HTTPRequest(std::string str);

	HTTPMethod getMethod() { return method; }
	std::string getBody() { return body; }
	std::string getContextPath() { return contextPath; }
	std::string getOriginalRequestString() { return originalRequest; }
	std::string getQueryParam(std::string param) { return queryParams[param]; }
};

class HTTPResponse {
private:
	int status;
	time_t time;
	int contentLength; //in bytes
	string body;
	map<string, string> headers;

	map<int, string> statusCodes = {
	{200,"200 OK"}, {404,"404 Not Found"},{412,"412 Precondition failed"},{201,"201 Created"}
	,{204,"204 No Content"},{501,"501 Not Implemented"}
	};


public:
	HTTPResponse(int status, string body) {
		this->status = status;
		this->body = body;
		this->contentLength = body.length();
		this->time = std::time(&time);
	}

	void addHeader(const string& key, const string& value) {
		headers[key] = value;
	}

	string createString() {
		string headerString = "";

		for (const auto& element : headers) {
			headerString += element.first + ": " + element.second + "\n";
		}

		string str = "HTTP/1.1 " + statusCodes[status] + "\n"
			+ headerString 
			+ "Date: " + ctime(&time)
			+ "Content-Length: " + to_string(contentLength) + "\n"
			+ "\n"
			+ body;

		return str;
	}
};