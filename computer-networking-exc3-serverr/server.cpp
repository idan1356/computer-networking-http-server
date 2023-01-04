#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <cstring>
#include <regex>

#include <time.h>
#include "request_utils.h"


struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	HTTPRequest* sendSubType;	// Sending sub-type
	char buffer[2048];
	int len;
};

const int TIME_PORT = 27015;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);
string extractRequestFromBuffer(char* buffer);

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

void main()
{
	cout << "starting server";
	WSAData wsaData;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(TIME_PORT);

	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN);

	while (true)
	{
		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(i);
					break;
				}
			}
		}
	}

	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;

		if (sockets[index].len > 0)
		{
			sockets[index].send = SEND;
			string curRequest = extractRequestFromBuffer(sockets[index].buffer);
			sockets[index].sendSubType = new HTTPRequest(curRequest);
		}
	}
}

void sendMessage(int index)
{
	int bytesSent = 0;
	char sendBuff[2048];
	string responseString;
	SOCKET msgSocket = sockets[index].id;
	if (sockets[index].sendSubType->getMethod() == HTTPRequest::GET_OR_HEAD)
	{
		HTTPResponse response = GET_OR_HEAD(sockets[index].sendSubType);
		responseString = response.createString();
	}
	else if (sockets[index].sendSubType->getMethod() == HTTPRequest::PUT)
	{
		HTTPResponse response = PUT(sockets[index].sendSubType);
		responseString = response.createString();
	}
	else if (sockets[index].sendSubType->getMethod() == HTTPRequest::DELETE_)
	{
		HTTPResponse response = DELETE_(sockets[index].sendSubType);
		responseString = response.createString();
	}
	else if (sockets[index].sendSubType->getMethod() == HTTPRequest::HEAD)
	{
		HTTPResponse response = GET_OR_HEAD(sockets[index].sendSubType, true);
		responseString = response.createString();
	}
	else if (sockets[index].sendSubType->getMethod() == HTTPRequest::TRACE)
	{
		HTTPResponse response = TRACE(sockets[index].sendSubType);
		responseString = response.createString();
	}
	else if (sockets[index].sendSubType->getMethod() == HTTPRequest::OPTIONS)
	{
		HTTPResponse response = OPTIONS(sockets[index].sendSubType);
		responseString = response.createString();
	}
	else if (sockets[index].sendSubType->getMethod() == HTTPRequest::POST)
	{
		HTTPResponse response = POST(sockets[index].sendSubType);
		responseString = response.createString();
	}

	memcpy(sendBuff, responseString.c_str() + '\0', responseString.size() + 1);
	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);

	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
	closesocket(msgSocket);
	removeSocket(index);
	sockets[index].send = IDLE;
}


string extractRequestFromBuffer(char* buffer) {
	char* separator = strstr(buffer, "\r\n\r\n");
	if (separator == NULL) {
		std::cerr << "Error: Separator not found" << std::endl;
		return buffer;
	}

	std::string request(buffer, separator - buffer);
	std::regex content_length_regex("Content-Length: (\\d+)");
	std::smatch content_length_match;

	if (std::regex_search(request, content_length_match, content_length_regex)) {
		int content_length = std::stoi(content_length_match[1]);
		char* body = separator + 4;
		string a = std::string(buffer, body + content_length - buffer);
		// Move the remainder of the buffer to the beginning
		size_t remaining = strlen(body + content_length);
		memmove(buffer, body + content_length, remaining);
		memset(buffer + remaining, 0, sizeof(buffer) - remaining);
		return a;
	}

	// Move the remainder of the buffer to the beginning
	size_t remaining = strlen(separator + 4);
	memmove(buffer, separator + 4, remaining);
	memset(buffer + remaining, 0, sizeof(buffer) - remaining);

	return request;
}



