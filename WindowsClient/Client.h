#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <string>

class Client
{
public:
	void ConnectToServer(SOCKET& clientSocket, const std::string& serverIP, int port);
	void NonBlockingCommunication(SOCKET& clientSocket);
	void sendUsername(SOCKET& clientSocket, fd_set& readSet);
};