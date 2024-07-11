#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>

class TcpServer
{
public:
	TcpServer();
	void startServer();

private:
	boost::asio::io_context _context;
	boost::thread _acceptThread;
	unsigned int _port;
	unsigned int _daemonPort;
	std::string _daemonIp;

	void CreateAcceptThread();
	void SendCommandToDaemon(const std::string& command);
	void SendDataToClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& message);
	void ReadDataFromClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
};