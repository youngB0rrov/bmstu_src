#include "TcpServer.h"
#include "../../Utils/ConfigHelper/ConfigHelper.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include "../../Utils/Logger/Logger.h"
#include "../../Utils/CommandsHelper/CommandsHelper.h"
#include <sstream>
#include <unordered_map>
#include <boost/uuid/string_generator.hpp>

TcpServer::TcpServer()
{
    int port, daemonPort, serversListenPort;
    std::string daemonIp;
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Network.listenPort", port);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "DaemonNetwork.daemonPort", daemonPort);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "DaemonNetwork.daemonIp", daemonIp);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Network.serversListenPort", serversListenPort);
    _port = port;
    _daemonPort = daemonPort;
    _daemonIp = daemonIp;
    _serversListenPort = serversListenPort;
}

void TcpServer::StartServer()
{
    Logger::GetInstance() << "Start listening requests on 0.0.0.0:" << _port << std::endl;
    _clientsAcceptThread = boost::thread(&TcpServer::CreateClientsAcceptThread, this);
    _serversAcceptThread = boost::thread(&TcpServer::CreateServersAcceptThread, this);

    _clientsAcceptThread.join();
    _serversAcceptThread.join();
}

void TcpServer::CreateClientsAcceptThread()
{
    // Инициализация эндпоинта хоста, куда будут подключаться клиенты
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), _port);

    // Инициализация сокета для прослушки входящих соединений
    boost::asio::ip::tcp::acceptor acceptor(_context, endpoint);

    while (true)
    {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        boost::shared_ptr<boost::asio::ip::tcp::socket> clientSocket(new boost::asio::ip::tcp::socket(_context));
        acceptor.accept(*clientSocket);
        boost::thread(boost::bind(&TcpServer::ReadDataFromClientSocket, this, clientSocket));
    }
}

void TcpServer::CreateServersAcceptThread()
{
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), _serversListenPort);
    boost::asio::ip::tcp::acceptor acceptor(_context, endpoint);
    //Logger::GetInstance() << "Start listening server requests on 0.0.0.0:" << _serversListenPort << std::endl;

    while (true)
    {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        boost::shared_ptr<boost::asio::ip::tcp::socket> serversSocket(new boost::asio::ip::tcp::socket(_context));
        acceptor.accept(*serversSocket);
        boost::thread(boost::bind(&TcpServer::ReadDataFromServerSocket, this, serversSocket));
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

void TcpServer::ReadDataFromClientSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    bool bIsReading(true);
    while (bIsReading)
    {
        char data[512];

        size_t bytesRead = socket->read_some(boost::asio::buffer(data));
        if (bytesRead > 0)
        {
            std::string message = std::string(data, bytesRead);
            ProcessDataFromClient(message, socket);
            bIsReading = false;
        }
    }
}

void TcpServer::ReadDataFromServerSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    try
    {
        while (true)
        {
            char data[512];

            size_t bytesRead = socket->read_some(boost::asio::buffer(data));
            if (bytesRead > 0)
            {
                std::string message = std::string(data, bytesRead);
                ProcessDataFromServer(message, socket);
            }
        }
    }
    catch (const boost::system::system_error& error)
    {
        if (error.code() == boost::asio::error::eof || error.code() == boost::asio::error::connection_reset)
        {
            std::cout << "Finish reading from client socket, client disconnected cleanly" << std::endl;
        }
        else
        {
            std::cout << "Socket error occured: " << error.what() << std::endl;
        }
    }

    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket->close();
    std::cout << "Client socket closed" << std::endl;
}

void TcpServer::ProcessDataFromClient(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    std::cout << "Got data from client: " << message << std::endl;
    
    ClientInfo clientInfo;
    clientInfo.Socket = socket;
    
    if (message == "CREATE")
    {
        clientInfo.UserType = ClientType::INITIATOR;
        SendCommandToDaemon(std::string("START"));
    }
    if (message == "JOIN")
    {
        clientInfo.UserType = ClientType::PLAYER;
    }
    
    _connectedClients.push_back(clientInfo);
    std::cout << "Added client to queue: [CLIENT_ADDRESS=" << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << ", CLIENT_TYPE=" << clientInfo.UserType << "]" << std::endl;
}

void TcpServer::ProcessDataFromServer(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    Logger::GetInstance() << "Got data from dedicated server: " << message << std::endl;

    std::string commandType = CommandsHelper::GetCommandTypeFromMessage(message);
    std::unordered_map<std::string, std::string> commandKeyValuePairs = CommandsHelper::GetKeyValuePairs(message);

    if (commandType == "REGISTER_SERVER")
    {
        Logger::GetInstance() << "Registering server job started..." << std::endl;
        ServerInfo newServer;

        newServer.m_uuid = commandKeyValuePairs["uuid"];
        newServer.m_URI = commandKeyValuePairs["uri"];
        newServer.m_maxPlayers = atoi(commandKeyValuePairs["max_players"].c_str());
        newServer.m_currentPlayers = atoi(commandKeyValuePairs["current_players"].c_str());

        _runningServers.insert(_runningServers.begin(), newServer);
        std::cout << "Registered server with uuid = " << newServer.m_uuid << std::endl;
        SendConnectionStringToClient(newServer.m_URI);

        Logger::GetInstance() << "Registering server job finished..." << std::endl;
    }
    if (commandType == "UPDATE_SERVER")
    {
        Logger::GetInstance() << "Updating server job started..." << std::endl;
        std::string updatedInstanceUuid = commandKeyValuePairs["uuid"];

        auto registeredServer = boost::find_if(_runningServers, [&updatedInstanceUuid](const ServerInfo& serverInfo)
        {
            return serverInfo.m_uuid == updatedInstanceUuid;
        });

        if (registeredServer == _runningServers.end())
        {
            std::cout << "Server with such uuid: " << updatedInstanceUuid << " not found" << std::endl;
            Logger::GetInstance() << "Updating server job finished..." << std::endl;
            
            return;
        }

        ServerInfo& foundRegisteredServer(*registeredServer);
        int currentPlayers = atoi(commandKeyValuePairs["current_players"].c_str());

        std::cout << "Old value: " << foundRegisteredServer.m_currentPlayers << std::endl;
        foundRegisteredServer.m_currentPlayers = currentPlayers;
        std::cout << "New value: " << foundRegisteredServer.m_currentPlayers << std::endl;

        Logger::GetInstance() << "Updating server job finished..." << std::endl;
    }
}

void TcpServer::SendConnectionStringToClient(std::string& message)
{
    // Обработка присланного URI от DedicatedServer
    std::cout << "Sending uri of started dedicated server to client: " << message << std::endl;

    auto initiatorConnectedClients = boost::adaptors::filter(_connectedClients, [](const ClientInfo& clientInfo)
    {
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
