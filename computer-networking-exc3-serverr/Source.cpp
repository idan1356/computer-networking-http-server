#include "httplite.h"

void main() {
	HTTPResponse response = HTTPResponse(200, "TEST BODY");
	string responseStr = response.createString();

	string requestStr = "GET test/path\nHost: website.com\nTEST BODY";
	HTTPRequest request = HTTPRequest(stringstream(requestStr));
	
