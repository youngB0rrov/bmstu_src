#include <iostream>
#include "Engine/TcpServer/TcpServer.h"

int main()
{
	try
	{
		TcpServer* tcpServer = new TcpServer();
		tcpServer->startServer();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}