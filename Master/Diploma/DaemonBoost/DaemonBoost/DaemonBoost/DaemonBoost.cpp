#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/process.hpp>

void handleIncomeQuery(boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
void createAcceptThread();
void startServerInstance();

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
            const std::string message = std::string(data, bytesRead);
            std::cout << "Handle income query: " << message << std::endl;
            if (message == "Start")
            {
                std::cout << "Starting server instance..." << std::endl;
                boost::thread(startServerInstance).detach();
            }
            bIsReading = false;
        }
    }
}
void startServerInstance()
{
    #ifdef _WIN32
        const std::string scriptPath = "E:\\Maga\\sem2\\MMAPS\\Releases\\WindowsServer\\Lab4ServerPackaged.bat";
    #else
        const std::string scriptPath = "";
    #endif // _WIN32
    try
    {
        boost::process::child childThreat(scriptPath, boost::process::std_out > stdout, boost::process::std_err > stderr);
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