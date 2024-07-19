#pragma once
#include <boost/asio.hpp>
#include "../Data/Enums/ClientType.h"

struct ClientInfo
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> Socket;
	ClientType UserType;
};