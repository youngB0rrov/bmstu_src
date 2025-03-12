#include "../Engine/ConsoleMonitoring/ConsoleMonitoring.h"
#include "../Engine/TcpServer/TcpServer.h"
#include <string>
#include <boost/shared_ptr.hpp>

class Application
{

public:
	Application();
	~Application();

	void Run();

private:
	
	boost::shared_ptr<TcpServer> m_tcpServer;
	boost::shared_ptr<ConsoleMonitoring> m_consoleMonitoring;

	// Логирование
	std::string m_logPath;
};