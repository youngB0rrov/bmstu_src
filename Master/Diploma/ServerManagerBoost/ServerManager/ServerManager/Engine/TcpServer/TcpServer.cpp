#include "TcpServer.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <sstream>
#include <mutex>
#include <unordered_map>
#include <boost/uuid/string_generator.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include "../../Utils/ConfigHelper/ConfigHelper.h"
#include "../../Utils/Logger/Logger.h"
#include "../../Utils/CommandsHelper/CommandsHelper.h"
#include "../../Data/Enums/ServerCommandType.h"

TcpServer::TcpServer()
{
    int port, daemonPort, serversListenPort;
    std::string daemonIp;
    std::string logPath;

    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Network.listenPort", port);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "DaemonNetwork.daemonPort", daemonPort);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "DaemonNetwork.daemonIp", daemonIp);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Network.serversListenPort", serversListenPort);
    ConfigHelper::ReadVariableFromConfig("appsettings.ini", "Logger.logPath", logPath);

    _port = port;
    _daemonPort = daemonPort;
    _daemonIp = daemonIp;
    _serversListenPort = serversListenPort;
    _logPath = logPath;
}

void TcpServer::StartServer()
{
    try
    {
        Logger::GetInstance().SetLogFile(_logPath);
    }
    catch (std::runtime_error ex)
    {
        Logger::GetInstance() << std::string(ex.what()) << std::endl;
    }

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
    Logger::GetInstance() << "Sending command to daemon: " << command << std::endl;
    try
    {
        boost::asio::ip::tcp::endpoint daemonEnpoint(boost::asio::ip::address::from_string(_daemonIp), _daemonPort);
        boost::asio::ip::tcp::socket daemonSocket(_context);

        daemonSocket.connect(daemonEnpoint);
        daemonSocket.write_some(boost::asio::buffer(command));
    }
    catch (const std::exception& e)
    {
        Logger::GetInstance() << "An exception occured: " << std::string(e.what()) << std::endl;
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
            Logger::GetInstance() << "Finish reading from client socket, client disconnected cleanly" << std::endl;
        }
        else
        {
            Logger::GetInstance() << "Socket error occured: " << error.what() << std::endl;
        }
    }

    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket->close();
    Logger::GetInstance() << "Client socket closed" << std::endl;
}

void TcpServer::ProcessDataFromClient(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    Logger::GetInstance() << "Got data from client: " << message << std::endl;
    
    ClientInfo clientInfo;
    clientInfo.Socket = socket;
    
    // Попытка захвата лока для _runningServers
    std::lock_guard<std::mutex> lock(_runningServersMutex);

    if (_runningServers.empty())
    {
        Logger::GetInstance() << "Sending \"START\" command to daemon due to empty running server instances array" << std::endl;

        clientInfo.UserType = ClientType::INITIATOR;
        SendCommandToDaemon(std::string("START"));
        _connectedClients.push_back(clientInfo);

        Logger::GetInstance() << "Added client to queue: [CLIENT_ADDRESS=" << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << ", CLIENT_TYPE=" << clientInfo.UserType << "]" << std::endl;

        return;
    }

    auto runningServerInstances = boost::adaptors::filter(_runningServers, [](const ServerInfo& serverInfo)
    {
        return serverInfo.m_currentPlayers != serverInfo.m_maxPlayers && serverInfo.m_currentPlayers > 0;
    });

    if (runningServerInstances.empty())
    {
        Logger::GetInstance() << "Sending \"START\" command to daemon due to all running server instances occupation" << std::endl;

        clientInfo.UserType = ClientType::INITIATOR;
        SendCommandToDaemon(std::string("START"));
        _connectedClients.push_back(clientInfo);
        
        Logger::GetInstance() << "Added client to queue: [CLIENT_ADDRESS=" << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << ", CLIENT_TYPE=" << clientInfo.UserType << "]" << std::endl;

        return;
    }
    
    clientInfo.UserType = ClientType::PLAYER;
    auto& runningServerWithMostPlayers = *boost::range::max_element(runningServerInstances, [](const ServerInfo& A, const ServerInfo& B)
    {
        return A.m_currentPlayers < B.m_currentPlayers;
    });

    Logger::GetInstance() << "Found server instance [uuid=" << runningServerWithMostPlayers.m_uuid << "] for client [CLIENT_ADDRESS=" << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << ", CLIENT_TYPE=" << clientInfo.UserType << "]" << std::endl;
    Logger::GetInstance() << "Senging data: [URI=" << runningServerWithMostPlayers.m_URI << "] to cleint: [CLIENT_ADDRESS=" << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << ", CLIENT_TYPE=" << clientInfo.UserType << "] " << std::endl;
    SendDataToSocket(socket, runningServerWithMostPlayers.m_URI);
}

void TcpServer::ProcessDataFromServer(std::string& message, boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    Logger::GetInstance() << "Getting data from dedicated server: " << message << std::endl;

    ServerCommandType commandType = CommandsHelper::GetServerCommandType(message);
    std::unordered_map<std::string, std::string> commandKeyValuePairs = CommandsHelper::GetKeyValuePairs(message);

    switch (commandType)
    {
        case ServerCommandType::REGISTER_SERVER:
        {
            Logger::GetInstance() << "Registering server job started..." << std::endl;
            ServerInfo newServer;

            newServer.m_uuid = commandKeyValuePairs["uuid"];
            newServer.m_URI = commandKeyValuePairs["uri"];
            newServer.m_maxPlayers = atoi(commandKeyValuePairs["max_players"].c_str());
            newServer.m_currentPlayers = atoi(commandKeyValuePairs["current_players"].c_str());
            newServer.m_serverState = CommandsHelper::GetServerStateType(commandKeyValuePairs["state"].c_str());

            _runningServers.insert(_runningServers.begin(), newServer);
            Logger::GetInstance() << "Registered server with uuid = " << newServer.m_uuid << std::endl;
            SendConnectionStringToClient(newServer.m_URI);

            Logger::GetInstance() << "Registering server job finished..." << std::endl;
            break;
        }
        case ServerCommandType::UPDATE_SERVER:
        {
            Logger::GetInstance() << "Updating server job started..." << std::endl;
            std::string updatedInstanceUuid = commandKeyValuePairs["uuid"];

            auto registeredServer = boost::find_if(_runningServers, [&updatedInstanceUuid](const ServerInfo& serverInfo)
            {
                return serverInfo.m_uuid == updatedInstanceUuid;
            });

            if (registeredServer == _runningServers.end())
            {
                Logger::GetInstance() << "Server with such uuid: " << updatedInstanceUuid << " not found" << std::endl;
                Logger::GetInstance() << "Updating server job finished..." << std::endl;

                return;
            }

            ServerInfo& foundRegisteredServer(*registeredServer);
            int currentPlayers = atoi(commandKeyValuePairs["current_players"].c_str());
            ServerState currentServerState = CommandsHelper::GetServerStateType(commandKeyValuePairs["state"].c_str());

            Logger::GetInstance() << "Old value: [current_players = " << foundRegisteredServer.m_currentPlayers << ", state = " << foundRegisteredServer.m_serverState << "]" << std::endl;
            foundRegisteredServer.m_currentPlayers = currentPlayers;
            foundRegisteredServer.m_serverState = currentServerState;
            Logger::GetInstance() << "New value: [current_players = " << foundRegisteredServer.m_currentPlayers << ", state = " << foundRegisteredServer.m_serverState << "]" << std::endl;

            Logger::GetInstance() << "Updating server job finished..." << std::endl;
            break;
        }
        default:
            Logger::GetInstance() << "Unknown command type from server, skip processing data..." << std::endl;
    }
}

void TcpServer::SendConnectionStringToClient(std::string& message)
{
    // Обработка присланного URI от DedicatedServer
    Logger::GetInstance() << "Sending uri of started dedicated server to client: " << message << std::endl;

    auto initiatorConnectedClients = boost::adaptors::filter(_connectedClients, [](const ClientInfo& clientInfo)
    {
        return clientInfo.UserType == ClientType::INITIATOR;
    });

    if (initiatorConnectedClients.empty())
    {
        Logger::GetInstance() << "Connected clients queue is empty. No client to send IP:PORT to" << std::endl;
        return;
    }

    ClientInfo& firstInitiatorInQueue = initiatorConnectedClients.front();
    Logger::GetInstance() << "Senging data to cleint: " << message << std::endl;
    SendDataToSocket(firstInitiatorInQueue.Socket, message);
    _connectedClients.erase(_connectedClients.begin());
}
