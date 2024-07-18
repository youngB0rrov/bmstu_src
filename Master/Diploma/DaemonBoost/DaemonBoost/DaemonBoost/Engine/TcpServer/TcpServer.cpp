#include <boost/process.hpp>
#include <iostream>
#include "TcpServer.h"
#include "../../Utils/ConfigHelper/ConfigHelper.h"

TcpServer::TcpServer()
{
    int port;
    std::string scriptPath;
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Network.listenPort", port);
    _port = port;

    #ifdef _WIN32
        ConfigHelper::ReadVariableFromConfig("appsettings.ini", "SystemPaths.scriptPathWindows", scriptPath);
    #else
        ConfigHelper::ReadVariableFromConfig("appsettings.ini", "SystemPaths.scriptPathLinux", scriptPath);
    #endif // _WIN32
    _scriptPath = scriptPath;
}

void TcpServer::StartServer()
{
    _acceptThread = boost::thread(&TcpServer::CreateAcceptThread, this);
    _acceptThread.join();
}

void TcpServer::CreateAcceptThread()
{
    // Инициализация эндпоинта хоста, куда будут подключаться клиенты
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), _port);

    // Инициализация сокета для прослушки входящих соединений
    boost::asio::ip::tcp::acceptor acceptor(_context, endpoint);
    std::cout << "Start listening on 0.0.0.0:" << _port << std::endl;

    while (true)
    {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        boost::shared_ptr<boost::asio::ip::tcp::socket> clientSocket(new boost::asio::ip::tcp::socket(_context));
        acceptor.accept(*clientSocket);
        boost::thread(boost::bind(&TcpServer::HandleIncomeQuery, this, clientSocket));
    }
}

void TcpServer::HandleIncomeQuery(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    bool bIsReading(true);
    while (bIsReading)
    {
        char data[512];

        size_t bytesRead = socket->read_some(boost::asio::buffer(data));
        if (bytesRead > 0)
        {
            const std::string message = std::string(data, bytesRead);
            std::cout << "Handle income query: " << message << std::endl;
            if (message == "Start")
            {
                std::cout << "Starting server instance..." << std::endl;
                boost::thread(&TcpServer::StartServerInstance, this).detach();
            }
            bIsReading = false;
        }
    }
}

void TcpServer::StartServerInstance()
{
    try
    {
        boost::process::child childThreat(_scriptPath, boost::process::std_out > stdout, boost::process::std_err > stderr);
        childThreat.wait();
        if (childThreat.exit_code() == 0)
        {
            std::cout << "Server instance started successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Error, while starting server instance. Exit code: " << childThreat.exit_code() << std::endl;
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Exception occured, while starting server instance: " << exception.what() << std::endl;
    }
}
