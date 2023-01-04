#include "request_utils.h"

HTTPResponse GET_OR_HEAD(HTTPRequest* request, bool head) {
	string buffer, textFromFile;
	string url = request->getContextPath();
	string lang = request->getQueryParam("lang");

	std::string html = "";
	string a = url + lang + ".html";
	std::fstream html_file(url + lang + ".html", std::ios::in);
	int statusCode = 200;

	if (!html_file.is_open()) {
		std::cout << "Unable to open file" << std::endl;
		statusCode = 404;
	}

	if (!head) {
		std::string line;
		while (getline(html_file, line)) {
			html += line + '\n';
		}
	}

	html_file.close();
	return HTTPResponse(statusCode, html);
}

HTTPResponse PUT(HTTPRequest* request) {
	int statusCode = 200;
	string url = request->getContextPath();
	fs::path filePath(url);

	try {
		fs::create_directories(filePath.parent_path());
	}
	catch (std::exception& e) {
		statusCode = 501;
		return HTTPResponse(statusCode, "could not create a directory");
	}

	statusCode = fs::exists(filePath) ? 200 : 201;

	std::ofstream file(filePath, std::ios::out | std::ios::app); // open in append mode
	if (!file.is_open()) {
		statusCode = 501;
		return HTTPResponse(statusCode, "could not create file");
	}

	statusCode = fs::is_empty(filePath) && statusCode == 200 ? 204 : statusCode;

	file << request->getBody();
	file.close();

	return HTTPResponse(statusCode, "file uploaded successfully");
}

HTTPResponse DELETE_(HTTPRequest* request) {
	string url = request->getContextPath();
	fs::path filePath(url);
	return fs::remove(filePath) ? HTTPResponse(200, "the file removed successfully") : HTTPResponse(204, "the file was not removed");
}

HTTPResponse POST(HTTPRequest* request) {
	string body = request->getBody();
	cout << "message posted to server from client:" << request->getQueryParam("Host") << endl << body << endl;
	return HTTPResponse(200, "resource ");
}

HTTPResponse TRACE(HTTPRequest* request) {
	return HTTPResponse(200, request->getOriginalRequestString());
}


HTTPResponse OPTIONS(HTTPRequest* request) {
	HTTPResponse response =  HTTPResponse(200, "");
	response.addHeader("Allow", "GET PUT TRACE DELETE HEAD OPTIONS POST");
	return response;
}

