#pragma once

#include <cstdint>
#include <string>
#include "../Enums/ServerState.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#pragma pack(push, 1)
struct ServerUpdateMessage
{
	//char m_uuid[37];
	uint8_t m_uuid[16];
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

		boost::uuids::uuid appUuid;
		memcpy(&appUuid, raw.m_uuid, 16);

		serverUpdateStruct.m_uuid = boost::uuids::to_string(appUuid);
		serverUpdateStruct.m_currentPlayers = raw.m_currentPlayers;
		serverUpdateStruct.m_serverState = static_cast<ServerState>(raw.m_serverState);

		return serverUpdateStruct;
	}
};