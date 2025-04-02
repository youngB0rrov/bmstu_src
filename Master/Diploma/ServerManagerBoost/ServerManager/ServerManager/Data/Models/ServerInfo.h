#include <string>
#include "../Enums/ServerState.h"
#include "../Network/ServerRegisterMessage.h";

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

		serverInfo.m_uuid = std::string(raw.m_uuid);
		serverInfo.m_URI = std::string(raw.m_uri);
		serverInfo.m_currentPlayers = raw.m_currentPlayers;
		serverInfo.m_maxPlayers = raw.m_maxPlayers;
		serverInfo.m_serverState = static_cast<ServerState>(raw.m_serverState);

		return serverInfo;
	}
};