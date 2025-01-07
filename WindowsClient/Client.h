#pragma once
#include <winsock2.h>
#include <string>

class Client
{
public:
	void ConnectToServer(SOCKET& clientSocket, const std::string& serverIP, int port);
	void NonBlockingCommunication(SOCKET& clientSocket);
};