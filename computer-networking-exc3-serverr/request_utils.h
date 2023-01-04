#pragma once
#include "http.h"
#include "string_utils.h"
#include <iostream>
#include <fstream>
using namespace std;

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

HTTPResponse GET_OR_HEAD(HTTPRequest* request, bool head = false);
HTTPResponse PUT(HTTPRequest* request);
HTTPResponse DELETE_(HTTPRequest* request);
HTTPResponse TRACE(HTTPRequest* request);
HTTPResponse OPTIONS(HTTPRequest* request);
HTTPResponse POST(HTTPRequest* request);