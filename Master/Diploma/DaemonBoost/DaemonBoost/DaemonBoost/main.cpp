#include <iostream>
#include "Engine/TcpServer/TcpServer.h"

int main()
{
	TcpServer* tcpServer = new TcpServer();
	tcpServer->StartServer();
	
	delete tcpServer;
	return 0;
}