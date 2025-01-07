#include "Client.h"
#include "SocketUtil.h"

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    InitializeWinsock();

    SOCKET clientSocket = CreateNonBlockingSocket(SOCK_STREAM);

    Client client;
    client.ConnectToServer(clientSocket, SERVERIP, PORT);
    client.NonBlockingCommunication(clientSocket);

    Cleanup(clientSocket);

    return 0;
}