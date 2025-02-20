#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class TcpServer
{
public:
	TcpServer();
	void StartServer();

private:
	boost::asio::io_context _context;
	boost::thread _acceptThread;

	unsigned int _port;

	// �����������
	std::string _logPath;

	// ���������� �� �������
	std::string _scriptPath;

	void CreateAcceptThread();
	void HandleIncomeQuery(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void StartServerInstance();
};
