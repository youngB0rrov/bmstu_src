#pragma once

#include <string>
#include <cstdint>
#include <string>
#include "../Enums/ServerState.h"
#include "../Network/ServerNetworkProtocol.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

struct ServerUpdateStruct
{
	std::string m_uuid;
	int m_currentPlayers;
	ServerState m_serverState;

	ServerUpdateStruct(): m_uuid(""), m_currentPlayers(0), m_serverState(ServerState::LOBBY) { }

	static ServerUpdateStruct FromRaw(const ServerNetworkProtocol::ServerUpdateMessage& raw)
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