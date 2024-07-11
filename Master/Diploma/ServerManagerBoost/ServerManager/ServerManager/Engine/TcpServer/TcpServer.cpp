#include "TcpServer.h"
#include "../../Utils/ConfigHelper/ConfigHelper.h"
#include <boost/date_time/posix_time/posix_time.hpp>

TcpServer::TcpServer()
{
    int port, daemonPort;
    std::string daemonIp;
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "listenPort", port);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "daemonPort", daemonPort);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "daemonIp", daemonIp);
    _port = port;
    _daemonPort = daemonPort;
    _daemonIp = daemonIp;
}

void TcpServer::startServer()
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
        boost::thread(boost::bind(&TcpServer::ReadDataFromClient, this, clientSocket));
    }
}

void TcpServer::SendCommandToDaemon(const std::string& command)
{
    try
    {
        boost::asio::ip::tcp::endpoint daemonEnpoint(boost::asio::ip::address::from_string(_daemonIp), _daemonPort);
        boost::asio::ip::tcp::socket daemonSocket(_context);

        daemonSocket.connect(daemonEnpoint);
        daemonSocket.write_some(boost::asio::buffer(command));
    }
    catch (const std::exception& e)
    {
        std::cout << "An exception occured: " << std::string(e.what()) << std::endl;
    }
}

void TcpServer::SendDataToClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& message)
{
    socket->write_some(boost::asio::buffer(message, message.length() + 1));
}

void TcpServer::ReadDataFromClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    bool bIsReading(true);
    while (bIsReading)
    {
        char data[512];

        size_t bytesRead = socket->read_some(boost::asio::buffer(data));
        if (bytesRead > 0)
        {
            std::string message = std::string(data, bytesRead);
            SendDataToClient(socket, "Request queued");
            std::cout << "Got data form client: " << message << std::endl;
            std::cout << "Sending command to daemon: " << "Start" << std::endl;
            SendCommandToDaemon(std::string("Start"));
            bIsReading = false;
        }
    }
}
