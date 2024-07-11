#include <iostream>
#include "Engine/TcpServer/TcpServer.h"

int main()
{
	try
	{
		TcpServer* tcpServer = new TcpServer();
		tcpServer->StartServer();
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occured: " << e.what() << std::endl;
	}
}