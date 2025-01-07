#include "Server.h"
#include "SocketUtil.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
#include <ws2tcpip.h>

Server::Server(int port) : m_port(port), m_serverSocket(NULL) {}

Server::~Server()
{
    Cleanup(m_serverSocket);
}

void Server::BindAndListen()
{
    m_serverSocket = CreateNonBlockingSocket(SOCK_STREAM);
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
    serverAddr.sin_port = htons(m_port);

    // Bind the socket
    if (bind(m_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        Cleanup(m_serverSocket);
        return;
    }

    // Start listening
    if (listen(m_serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
        Cleanup(m_serverSocket);
        return;
    }

    std::cout << "Server is listening on port " << m_port << "...\n";
}

void Server::Run()
{
    fd_set masterSet, readSet;
    FD_ZERO(&masterSet);

    // Add the server socket to the master set
    FD_SET(m_serverSocket, &masterSet);
    // Set maxSocket to the server socket initially
    int maxSocket = m_serverSocket;

    while (true)
    {
        readSet = masterSet; // Copy the master set
        int activity = select(maxSocket + 1, &readSet, nullptr, nullptr, nullptr);

        if (activity == SOCKET_ERROR)
        {
            std::cerr << "select() failed: " << WSAGetLastError() << "\n";
            break;
        }

        // Check for activity on the server socket (new connections)
        if (FD_ISSET(m_serverSocket, &readSet))
        {
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            SOCKET clientSocket = accept(m_serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSocket == INVALID_SOCKET)
            {
                std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
                continue;
            }

            // Add the new client socket to the master set
            FD_SET(clientSocket, &masterSet);
            maxSocket = std::max(maxSocket, static_cast<int>(clientSocket));

            // Send prompt to client
            const char* prompt = "[Server] Enter your username: ";
            send(clientSocket, prompt, strlen(prompt), 0);

            while (true)
            {
                if (FD_ISSET(m_serverSocket, &readSet))
                {
                    // Receive username
                    char buffer[BUFFER_SIZE];
                    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

                    if (bytesReceived > 0)
                    {
                        buffer[bytesReceived] = '\0';
                        m_clientUsernames[clientSocket] = std::string(buffer);

                        // Notify others
                        std::string joinMessage = m_clientUsernames[clientSocket] + " has joined the chat.\n";
                        std::cout << joinMessage;
                        BroadcastMessage(clientSocket, joinMessage);
                        break;
                    }
                    else if (bytesReceived == 0)
                    {
                        std::cerr << "Client disconnected before setting a username.\n";
                        closesocket(clientSocket);
                        FD_CLR(clientSocket, &masterSet);
                        break;
                    }
                    else
                    {
                        int error = WSAGetLastError();
                        if (error != WSAEWOULDBLOCK)
                        {
                            std::cerr << "Failed to receive username: " << error << "\n";
                            closesocket(clientSocket);
                            FD_CLR(clientSocket, &masterSet);
                            break;
                        }
                    }
                }                
            }            
        }

        // Handle activity on client sockets
        for (SOCKET clientSocket = 0; clientSocket <= maxSocket; ++clientSocket)
        {
            if (FD_ISSET(clientSocket, &readSet) && clientSocket != m_serverSocket)
            {
                char buffer[BUFFER_SIZE];
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

                if (bytesReceived > 0)
                {
                    buffer[bytesReceived] = '\0';

                    // Check if the message is a command
                    if (buffer[0] == '/')
                    {
                        HandleCommand(clientSocket, std::string(buffer));
                    }
                    else
                    {
                        // Broadcast the message
                        std::string message = m_clientUsernames[clientSocket] + ": " + buffer;
                        std::cout << message << std::endl;
                        BroadcastMessage(clientSocket, message);
                    }
                }
                else if (bytesReceived == 0)
                {
                    // Client disconnected
                    std::cout << m_clientUsernames[clientSocket] << " disconnected.\n";
                    std::string leaveMessage = m_clientUsernames[clientSocket] + " has left the chat.\n";

                    BroadcastMessage(clientSocket, leaveMessage);

                    closesocket(clientSocket);
                    FD_CLR(clientSocket, &masterSet);
                    m_clientUsernames.erase(clientSocket);
                }
                else
                {
                    std::cerr << "recv() failed: " << WSAGetLastError() << "\n";
                    closesocket(clientSocket);
                    FD_CLR(clientSocket, &masterSet);
                    m_clientUsernames.erase(clientSocket);
                }
            }
        }
    }

    // Cleanup all client sockets
    for (const auto& [socket, username] : m_clientUsernames)
    {
        closesocket(socket);
    }
}

void Server::PromptUsername(SOCKET clientSocket)
{
    const char* prompt = "Enter your username: ";
    send(clientSocket, prompt, strlen(prompt), 0);

    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';
        m_clientUsernames[clientSocket] = std::string(buffer);
        std::cout << "User connected: " << buffer << "\n";

        // Notify other users
        std::string message = buffer + std::string(" has joined the chat.\n");
        BroadcastMessage(clientSocket, message);
    }
    else
    {
        std::cerr << "Failed to receive username.\n";
        closesocket(clientSocket);
    }
}

void Server::HandleClient(SOCKET clientSocket)
{
    char buffer[BUFFER_SIZE];
    while (true)
    {
        // Receive data from the client
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0)
        {
            // Process the received data
            buffer[bytesReceived] = '\0'; // Null-terminate the data
            std::cout << "Received from client: " << buffer << "\n";

            // Respond to the client
            std::string response = "Message received: " + std::string(buffer);
            send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
        }
        else if (bytesReceived == 0)
        {
            // Client closed the connection
            std::cout << "Client disconnected gracefully.\n";
            break;
        }
        else
        {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
            {
                // No data available, continue waiting
                continue;
            }
            else
            {
                std::cerr << "recv() failed: " << error << "\n";
                break;
            }
        }
    }

    // Close the client socket after exiting the loop
    closesocket(clientSocket);
    std::cout << "Connection with client closed.\n";
}

void Server::HandleCommand(SOCKET clientSocket, const std::string& command)
{
    if (command == "/list")
    {
        std::string userList = "Connected users:\n";
        for (const auto& [socket, username] : m_clientUsernames)
        {
            userList += username + "\n";
        }
        send(clientSocket, userList.c_str(), userList.size(), 0);
    }
    else if (command.starts_with("/msg "))
    {
        size_t space = command.find(' ', 5);
        if (space != std::string::npos)
        {
            std::string targetUser = command.substr(5, space - 5);
            std::string message = command.substr(space + 1);

            auto it = std::find_if(m_clientUsernames.begin(), m_clientUsernames.end(),
                [&targetUser](const auto& pair) { return pair.second == targetUser; });

            if (it != m_clientUsernames.end())
            {
                send(it->first, message.c_str(), message.size(), 0);
            }
            else
            {
                std::string error = "User not found.\n";
                send(clientSocket, error.c_str(), error.size(), 0);
            }
        }
        else
        {
            std::string error = "Invalid /msg format. Use /msg <username> <message>\n";
            send(clientSocket, error.c_str(), error.size(), 0);
        }
    }
    else if (command == "/exit")
    {
        closesocket(clientSocket);
        m_clientUsernames.erase(clientSocket);
    }
    else
    {
        std::string error = "Unknown command.\n";
        send(clientSocket, error.c_str(), error.size(), 0);
    }
}

void Server::BroadcastMessage(SOCKET senderSocket, const std::string& message)
{
    for (const auto& [clientSocket, username] : m_clientUsernames)
    {
        if (clientSocket != senderSocket) // Skip the sender
        {
            send(clientSocket, message.c_str(), message.size(), 0);
        }
    }
}

#pragma region SingleClient
void Server::AcceptAndHandleClient()
{
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (true)
    {
        SOCKET clientSocket = accept(m_serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET)
        {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK)
            {
                // No incoming connection; continue non-blocking
                Sleep(100); // Avoid busy-waiting
                continue;
            }
            else
            {
                std::cerr << "Accept failed: " << err << "\n";
                return;
            }
        }

        // Print client info
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "Accepted connection from " << clientIP << ":" << ntohs(clientAddr.sin_port) << "\n";

        HandleClient(clientSocket);
    }
}
#pragma endregion
