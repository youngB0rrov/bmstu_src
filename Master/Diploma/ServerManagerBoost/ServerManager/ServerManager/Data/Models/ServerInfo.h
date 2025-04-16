#include <string>
#include <sstream>
#include "../Enums/ServerState.h"
#include "../Network/ServerRegisterMessage.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

struct ServerInfo
{
	std::string m_uuid;
	std::string m_URI;
	int m_currentPlayers;
	int m_maxPlayers;
	ServerState m_serverState;

	ServerInfo() : m_uuid(""), m_URI(""), m_currentPlayers(0), m_maxPlayers(0), m_serverState(ServerState::LOBBY)
	{ 
		
	}
	
	ServerInfo(const ServerInfo& serverInfo) : 
		m_uuid(serverInfo.m_uuid),
		m_URI(serverInfo.m_URI),
		m_currentPlayers(serverInfo.m_currentPlayers),
		m_maxPlayers(serverInfo.m_maxPlayers),
		m_serverState(serverInfo.m_serverState)
	{

	}

	static ServerInfo FromRaw(const ServerRegisterMessage& raw)
	{
		ServerInfo serverInfo;

		boost::uuids::uuid appUuid;
		memcpy(&appUuid, raw.m_uuid, 16);

		boost::asio::ip::address_v4 addr(raw.m_ip);
		std::ostringstream oss;
		oss << addr.to_string() << ":" << raw.m_port;

		serverInfo.m_uuid = boost::uuids::to_string(appUuid);
		serverInfo.m_URI = oss.str();
		serverInfo.m_currentPlayers = raw.m_currentPlayers;
		serverInfo.m_maxPlayers = raw.m_maxPlayers;
		serverInfo.m_serverState = static_cast<ServerState>(raw.m_serverState);

		return serverInfo;
	}
};