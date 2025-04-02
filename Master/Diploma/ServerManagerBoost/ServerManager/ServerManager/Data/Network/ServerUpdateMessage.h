#pragma once

#include <cstdint>
#include <string>
#include "../Enums/ServerState.h"

#pragma pack(push, 1)
struct ServerUpdateMessage
{
	char m_uuid[37];
	uint16_t m_currentPlayers;
	uint8_t m_serverState;
};
#pragma pack(pop)

struct ServerUpdateStruct
{
	std::string m_uuid;
	int m_currentPlayers;
	ServerState m_serverState;

	static ServerUpdateStruct FromRaw(const ServerUpdateMessage& raw)
	{
		ServerUpdateStruct serverUpdateStruct;

		serverUpdateStruct.m_uuid = std::string(raw.m_uuid);
		serverUpdateStruct.m_currentPlayers = raw.m_currentPlayers;
		serverUpdateStruct.m_serverState = static_cast<ServerState>(raw.m_serverState);

		return serverUpdateStruct;
	}
};