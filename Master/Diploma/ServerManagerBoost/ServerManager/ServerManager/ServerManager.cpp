#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void sendDataToClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& message);
void readDataFromClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
void createAcceptThread();

int main()
{
    boost::thread_group threads;

    threads.create_thread(createAcceptThread);
    threads.join_all();

    return 0;
}
void createAcceptThread()
{
    const unsigned int port = 8870;
    // Создание контекста
    boost::asio::io_context context;

    // Инициализация эндпоинта хоста, куда будут подключаться клиенты
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    // Инициализация сокета для прослушки входящих соединений
    boost::asio::ip::tcp::acceptor acceptor(context, endpoint);
    std::cout << "Start listening on 0.0.0.0:" << port << std::endl;

    while (true)
    {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        boost::shared_ptr<boost::asio::ip::tcp::socket> clientSocket(new boost::asio::ip::tcp::socket(context));
        acceptor.accept(*clientSocket);
        boost::thread(boost::bind(readDataFromClient, clientSocket));
    }
}
void sendCommandToDaemon(const std::string& command)
{
    try
    {
        // Создание контекста
        boost::asio::io_context context;
        boost::asio::ip::tcp::endpoint daemonEnpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8871);
        boost::asio::ip::tcp::socket daemonSocket(context);

        daemonSocket.connect(daemonEnpoint);
        daemonSocket.write_some(boost::asio::buffer(command));
    }
    catch (const std::exception&)
    {
        std::cout << "Unhandled exception occured!" << std::endl;
    }
}
void sendDataToClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& message)
{
    socket->write_some(boost::asio::buffer(message, message.length() + 1));
}
void readDataFromClient(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    bool bIsReading(true);
    while (bIsReading)
    {
        char data[512];

        size_t bytesRead = socket->read_some(boost::asio::buffer(data));
        if (bytesRead > 0)
        {
            sendDataToClient(socket, "Request queued");
            std::cout << "Got data form client: " << std::string(data, bytesRead) << std::endl;
            std::cout << "Sending command to daemon..." << std::endl;
            sendCommandToDaemon(std::string("Start"));
            bIsReading = false;
        }
    }
}