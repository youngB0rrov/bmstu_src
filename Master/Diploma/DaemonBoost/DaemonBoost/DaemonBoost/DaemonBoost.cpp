#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void handleIncomeQuery(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
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
    const unsigned int port = 8871;
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
        boost::thread(boost::bind(handleIncomeQuery, clientSocket));
    }
}
void handleIncomeQuery(boost::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    bool bIsReading(true);
    while (bIsReading)
    {
        char data[512];

        size_t bytesRead = socket->read_some(boost::asio::buffer(data));
        if (bytesRead > 0)
        {
            std::cout << "Handle income query: " << std::string(data, bytesRead) << std::endl;
            bIsReading = false;
        }
    }
}
