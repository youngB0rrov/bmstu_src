#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct ServerRegisterMessage
{
	//char m_uri[32];
	//char m_uuid[37];
	uint32_t m_ip;
	uint16_t m_port;
	uint8_t m_uuid[16];
	uint16_t m_currentPlayers;
	uint16_t m_maxPlayers;
	uint8_t m_serverState;
};
#pragma pack(pop)