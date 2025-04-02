#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct MessageFrameHeader
{
	uint8_t m_commandType;
	uint16_t m_payloadSize;
};
#pragma pack(pop)