#include "Application.h"
#include "boost/thread.hpp";
#include "../Utils/Logger/Logger.h"
#include "../Utils/ConfigHelper/ConfigHelper.h"
#include <string>

Application::Application()
{
	m_tcpServer = boost::shared_ptr<TcpServer>(new TcpServer());
	m_consoleMonitoring = boost::shared_ptr<ConsoleMonitoring>(new ConsoleMonitoring(m_tcpServer.get()));

	std::string logPath;
	ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Logger.logPath", logPath);
	m_logPath = logPath;
}

Application::~Application()
{
	/*delete m_tcpServer;
	delete m_consoleMonitoring;*/
}

void Application::Run()
{
	try
	{
		Logger::GetInstance().SetLogFile(m_logPath);
	}
	catch (std::runtime_error ex)
	{
		Logger::GetInstance() << std::string(ex.what()) << std::endl;
	}

	Logger::GetInstance() << "Starting applicaton..." << std::endl;

	boost::thread serverThread = boost::thread(&TcpServer::StartServer, m_tcpServer.get());
	boost::thread consoleInterfaceThread = boost::thread(&ConsoleMonitoring::Run, m_consoleMonitoring.get());

	serverThread.join();
	consoleInterfaceThread.join();
}
