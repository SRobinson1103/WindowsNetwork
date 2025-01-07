#pragma once

#include <winsock2.h>

class Server
{
public:
	Server(int port = 8080) : m_port(port) {}

	void AcceptClient(SOCKET& serverSocket);
	void BindAndListen(SOCKET& serverSocket);
	void HandleClient(SOCKET clientSocket);

private:
	int m_port;
};
