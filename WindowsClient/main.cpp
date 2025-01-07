#include "Client.h"
#include "SocketUtil.h"

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include "Client.h"
#include "SocketUtil.h"

int main()
{
    InitializeWinsock();

    SOCKET clientSocket = CreateNonBlockingSocket(SOCK_STREAM);

    Client client;

    // Connect to the server
    client.ConnectToServer(clientSocket, SERVERIP, PORT);

    // Start the non-blocking communication loop
    client.NonBlockingCommunication(clientSocket);

    Cleanup(clientSocket);

    return 0;
}
