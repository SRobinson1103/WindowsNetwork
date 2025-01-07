#include "Server.h"
#include "SocketUtil.h"

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    // Initialize Winsock
    InitializeWinsock();

    // Create a non-blocking TCP socket
    SOCKET serverSocket = CreateNonBlockingSocket(SOCK_STREAM);

    Server server(PORT);

    // Bind and listen
    server.BindAndListen(serverSocket);
    // Accept and handle a client
    server.AcceptAndHandleClient(serverSocket);

    // Cleanup
    Cleanup(serverSocket);

    return 0;
}
