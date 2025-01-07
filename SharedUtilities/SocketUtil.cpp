#include "SocketUtil.h"

#include <iostream>

void InitializeWinsock()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << "\n";
    }
}

void Cleanup(SOCKET sock)
{
    closesocket(sock);
    WSACleanup();
}

SOCKET CreateNonBlockingSocket(int type)
{
    SOCKET sock = socket(AF_INET, type, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return NULL;
    }

    // Set the socket to non-blocking mode
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR)
    {
        std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << "\n";
        Cleanup(sock);
        return NULL;
    }

    return sock;
}