#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include <vector>
#include "../../ClientInfo.h"

class TcpServer
{
public:
	TcpServer();
	void StartServer();

private:
	boost::asio::io_context _context;
	boost::thread _acceptThread;

	// Параметры подключения
	unsigned int _port;
	unsigned int _daemonPort;
	std::string _daemonIp;

	std::vector<ClientInfo> _connectedClients;

	void CreateAcceptThread();
	void SendCommandToDaemon(const std::string& command);
	void SendDataToSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& message);
	void ReadDataFromSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void ProcessDataFromClient(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void ProcessDataFromDaemon(std::string& message);
};