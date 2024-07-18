#pragma once
#include <boost/asio.hpp>
#include "ClientType.h"

struct ClientInfo
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> Socket;
	ClientType UserType;
};