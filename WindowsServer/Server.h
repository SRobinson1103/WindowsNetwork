#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <string>
#include <unordered_map>
#include <winsock2.h>


class Server
{
public:
	Server(int port = 8080);
	~Server();
	void Run();
	void AcceptAndHandleClient();
	void BindAndListen();
	void BroadcastMessage(SOCKET senderSocket, const std::string& message);
	void HandleClient(SOCKET clientSocket);
	void HandleCommand(SOCKET clientSocket, const std::string& command);
	void PromptUsername(SOCKET clientSocket);

private:
	int m_port;
	SOCKET m_serverSocket;
	std::unordered_map<SOCKET, std::string> m_clientUsernames;
};
