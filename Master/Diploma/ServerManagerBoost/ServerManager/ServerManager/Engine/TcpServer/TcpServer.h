#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include <vector>
#include <mutex>
#include "../../Data/Models/ClientInfo.h"
#include "../../Data/Models/ServerInfo.h"
#include "../../Data/Network/ServerNetworkProtocol.h"

class TcpServer
{
public:
	TcpServer();

	void StartServer();
	const std::vector<ServerInfo>& GetServerInstances() const { return _runningServers; }

private:
	boost::asio::io_context _context;
	boost::thread _clientsAcceptThread;
	boost::thread _serversAcceptThread;
	std::mutex _runningServersMutex;

	// ��������� �����������
	unsigned int _port;
	unsigned int _serversListenPort;
	unsigned int _daemonPort;
	std::string _daemonIp;

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
	void ProcessBinaryDataFromServer(const ServerNetworkProtocol::MessageFrameHeader& header, const char* payload, const size_t payloadSize);
	void SendConnectionStringToClient(std::string& message);
	static ServerNetworkProtocol::ServerRegisterMessage ParseServerRegisterMessage(const char* data, size_t size);
	static ServerNetworkProtocol::ServerUpdateMessage ParseServerUpdateMessage(const char* data, size_t size);
};