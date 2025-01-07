#pragma once

#include <string>
#include <winsock2.h>

const int BUFFER_SIZE = 1024;
const std::string SERVERIP = "127.0.0.1";
const int PORT = 8080;

void InitializeWinsock();
void Cleanup(SOCKET sock);
SOCKET CreateNonBlockingSocket(int type);