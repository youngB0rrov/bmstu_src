#pragma once

#include <cstdint>
#include <string>
#include "../Enums/ServerState.h"
#include "../Enums/ServerCommandType.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ServerNetworkProtocol
{
	#pragma pack(push, 1)
	struct MessageFrameHeader
	{
		uint8_t m_commandType;
		uint16_t m_payloadSize;
	};

	struct ServerUpdateMessage
	{
		uint8_t m_uuid[16];
		uint16_t m_currentPlayers;
		uint8_t m_serverState;
	};

	struct ServerRegisterMessage
	{
		uint32_t m_ip;
		uint16_t m_port;
		uint8_t m_uuid[16];
		uint16_t m_currentPlayers;
		uint16_t m_maxPlayers;
		uint8_t m_serverState;
	};
	#pragma pack(pop)

	// Строго соблюдать порядок
	constexpr size_t PackageSizes[] = { sizeof(ServerRegisterMessage), sizeof(ServerUpdateMessage) };

	constexpr size_t GetMaxSize(const size_t* array, size_t count)
	{
		size_t max = 0;

		for (size_t i = 0; i < count; ++i)
		{
			if (array[i] > max)
			{
				max = array[i];
			}
		}

		return max;
	}

	constexpr size_t MaxServerPackageSize = GetMaxSize(PackageSizes, sizeof(PackageSizes) / sizeof(size_t));
};