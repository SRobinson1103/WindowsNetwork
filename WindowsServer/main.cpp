#include "Server.h"
#include "SocketUtil.h"

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    // Initialize Winsock
    InitializeWinsock();

    Server server(PORT);

    // Bind and listen
    server.BindAndListen();
    // Accept and handle a client
    server.Run();

    return 0;
}
