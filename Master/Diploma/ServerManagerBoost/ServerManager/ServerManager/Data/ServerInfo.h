#include <string>

struct ServerInfo
{
	std::string m_uuid;
	std::string m_URI;
	int m_currentPlayers;
	int m_maxPlayers;

	ServerInfo() : m_uuid(""), m_URI(""), m_currentPlayers(0), m_maxPlayers(0)
	{ 
		
	}
	
	ServerInfo(const ServerInfo& serverInfo) : 
		m_uuid(serverInfo.m_uuid),
		m_URI(serverInfo.m_URI),
		m_currentPlayers(serverInfo.m_currentPlayers),
		m_maxPlayers(serverInfo.m_maxPlayers)
	{

	}
};