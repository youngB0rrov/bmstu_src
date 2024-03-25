#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

typedef tcp::endpoint Endpoint;
typedef tcp::socket Socket;
typedef boost::shared_ptr<Socket> socketPtr;
void sendDataToClient(socketPtr);
void readDataFromClient(socketPtr);

int main()
{
    // Создание контекста
    io_context context;

    // Инициализация эндпоинта хоста, куда будут подключаться клиенты
    Endpoint endpoint(ip::address::from_string("127.0.0.1"), 8870);

    // Инициализация сокета для прослушки входящих соединений
    tcp::acceptor acceptor(context, endpoint);
    cout << "Start listening on 127.0.0.1:8870" << endl;

    while (true)
    {
        boost::this_thread::sleep(1);
        socketPtr clientSocket(new tcp::socket(context));
        acceptor.accept(*clientSocket);
        boost::thread(boost::bind(readDataFromClient, clientSocket));
    }
}

void sendDataToClient(socketPtr socket)
{
    const string message = "Connected to server manager";
    socket->write_some(buffer(message, message.length() + 1));
}

void readDataFromClient(socketPtr socket)
{
    while (true)
    {
        boost::this_thread::sleep(1);
        char data[512];

        size_t bytesRead = socket->read_some(buffer(data));
        if (bytesRead > 0)
        {
            sendDataToClient(socket);
            cout << "Got data form client: " << string(data) << endl;
        }
    }
}