#pragma once
#include <boost/asio.hpp>
#include "../Enums/ClientType.h"

struct ClientInfo
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> Socket;
	ClientType UserType;
};