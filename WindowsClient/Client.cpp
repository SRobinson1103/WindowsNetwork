#include "Client.h"
#include "SocketUtil.h"

#include <conio.h>
#include <iostream>
#include <ws2tcpip.h>

void Client::ConnectToServer(SOCKET& clientSocket, const std::string& serverIP, int port)
{
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    int result = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR)
    {
        std::cerr << "Connection failed: " << WSAGetLastError() << "\n";
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            Cleanup(clientSocket);
        }       
        
        return;
    }

    std::cout << "Connected to server at " << serverIP << ":" << port << "\n";
}

void Client::NonBlockingCommunication(SOCKET& clientSocket)
{
    fd_set writeSet, readSet;
    char buffer[BUFFER_SIZE];
    std::string input;
    bool isConnected = false;

    while (true)
    {
        FD_ZERO(&writeSet);
        FD_ZERO(&readSet);

        if (!isConnected)
        {
            FD_SET(clientSocket, &writeSet); // Check for connection only if not connected
        }
        FD_SET(clientSocket, &readSet);

        timeval timeout = { 0, 100000 }; // 100ms timeout

        int activity = select(0, &readSet, &writeSet, nullptr, &timeout);
        if (activity == SOCKET_ERROR)
        {
            std::cerr << "select() failed: " << WSAGetLastError() << "\n";
            break;
        }

        // Check if the socket is ready for writing (connection completed)
        if (!isConnected && FD_ISSET(clientSocket, &writeSet))
        {
            std::cout << "Connected to the server!\n";
            isConnected = true;
        }

        // Check for incoming data
        if (isConnected && FD_ISSET(clientSocket, &readSet))
        {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0)
            {
                buffer[bytesReceived] = '\0'; // Null-terminate the received data
                std::cout << "Server: " << buffer << "\n";
            }
            else if (bytesReceived == 0)
            {
                std::cout << "Connection closed by server.\n";
                break;
            }
            else if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                std::cerr << "recv() failed: " << WSAGetLastError() << "\n";
                break;
            }
        }

        if (isConnected && _kbhit())
        {
            std::getline(std::cin, input);
            if (input == "exit")
            {
                std::cout << "Exiting...\n";
                break;
            }

            // Ensure the size does not exceed the int limit
            if (input.size() > INT_MAX)
            {
                std::cerr << "Error: Input size exceeds the maximum allowable size for send().\n";
                continue;
            }

            int bytesSent = send(clientSocket, input.c_str(), static_cast<int>(input.size()), 0);
            if (bytesSent == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
            {
                std::cerr << "send() failed: " << WSAGetLastError() << "\n";
                break;
            }
        }
    }

    closesocket(clientSocket);
}
