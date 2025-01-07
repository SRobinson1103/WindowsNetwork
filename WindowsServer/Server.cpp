#include "Server.h"
#include "SocketUtil.h"

#include <iostream>
#include <ws2tcpip.h>

void Server::BindAndListen(SOCKET& serverSocket)
{
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
    serverAddr.sin_port = htons(m_port);

    // Bind the socket
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        Cleanup(serverSocket);
        return;
    }

    // Start listening
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
        Cleanup(serverSocket);
        return;
    }

    std::cout << "Server is listening on port " << m_port << "...\n";
}

void Server::AcceptAndHandleClient(SOCKET& serverSocket)
{
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
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