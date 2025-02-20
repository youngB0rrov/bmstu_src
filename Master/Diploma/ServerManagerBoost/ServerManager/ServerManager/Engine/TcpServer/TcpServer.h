#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include <vector>
#include <mutex>
#include "../../Data/ClientInfo.h"
#include "../../Data/ServerInfo.h"

class TcpServer
{
public:
	TcpServer();
	void StartServer();

private:
	boost::asio::io_context _context;
	boost::thread _clientsAcceptThread;
	boost::thread _serversAcceptThread;
	std::mutex _runningServersMutex;

	// Параметры подключения
	unsigned int _port;
	unsigned int _serversListenPort;
	unsigned int _daemonPort;
	std::string _daemonIp;

	// Логирование
	std::string _logPath;

	std::vector<ClientInfo> _connectedClients;
	std::vector<ServerInfo> _runningServers;

	void CreateClientsAcceptThread();
	void CreateServersAcceptThread();
	void SendCommandToDaemon(const std::string& command);
	void SendDataToSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& message);
	void ReadDataFromClientSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void ReadDataFromServerSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void ProcessDataFromClient(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void ProcessDataFromServer(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void SendConnectionStringToClient(std::string& message);
};