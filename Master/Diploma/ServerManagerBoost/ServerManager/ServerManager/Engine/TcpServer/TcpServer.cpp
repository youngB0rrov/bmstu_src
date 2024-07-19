#include "TcpServer.h"
#include "../../Utils/ConfigHelper/ConfigHelper.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/range/adaptors.hpp>

TcpServer::TcpServer()
{
    int port, daemonPort;
    std::string daemonIp;
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Network.listenPort", port);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "DaemonNetwork.daemonPort", daemonPort);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "DaemonNetwork.daemonIp", daemonIp);
    _port = port;
    _daemonPort = daemonPort;
    _daemonIp = daemonIp;
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
        boost::thread(boost::bind(&TcpServer::ReadDataFromSocket, this, clientSocket));
    }
}

void TcpServer::SendCommandToDaemon(const std::string& command)
{
    std::cout << "Sending command to daemon: " << command << std::endl;
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

void TcpServer::SendDataToSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& message)
{
    socket->write_some(boost::asio::buffer(message, message.length() + 1));
}

void TcpServer::ReadDataFromSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    bool bIsReading(true);
    while (bIsReading)
    {
        char data[512];

        size_t bytesRead = socket->read_some(boost::asio::buffer(data));
        if (bytesRead > 0)
        {
            std::string message = std::string(data, bytesRead);
            if (message.find(':') != std::string::npos)
            {
                ProcessDataFromDaemon(message);
            }
            else
            {
                ProcessDataFromClient(message, socket);
            }
            bIsReading = false;
        }
    }
}

void TcpServer::ProcessDataFromClient(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    std::cout << "Got data from client: " << message << std::endl;
    
    ClientInfo clientInfo;
    clientInfo.Socket = socket;
    
    if (message == "CREATE")
    {
        clientInfo.UserType = ClientType::INITIATOR;
        SendCommandToDaemon(std::string("Start"));
    }
    if (message == "JOIN")
    {
        clientInfo.UserType = ClientType::PLAYER;
    }
    
    _connectedClients.push_back(clientInfo);
    std::cout << "Added client to queue: [CLIENT_ADDRESS=" << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << ",CLIENT_TYPE=" << clientInfo.UserType << "]" << std::endl;
}

void TcpServer::ProcessDataFromDaemon(std::string& message)
{
    // Обработка присланного URI от DedicatedServer
    std::cout << "Got URI form started DedicatedServer: " << message << std::endl;

    auto initiatorConnectedClients = boost::adaptors::filter(_connectedClients, [](const ClientInfo& clientInfo) {
        return clientInfo.UserType == ClientType::INITIATOR;
    });
    if (initiatorConnectedClients.empty())
    {
        std::cout << "Connected clients queue is empty. No client to send IP:PORT to" << std::endl;
        return;
    }

    ClientInfo& firstInitiatorInQueue = initiatorConnectedClients.front();
    std::cout << "Senging data to cleint: " << message << std::endl;
    SendDataToSocket(firstInitiatorInQueue.Socket, message);
    _connectedClients.erase(_connectedClients.begin());
}
