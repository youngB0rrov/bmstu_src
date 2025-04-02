#pragma once

#include <cstdlib>

enum class ServerCommandType: uint8_t
{
	REGISTER_SERVER,
	UPDATE_SERVER,
	UNKNOWN
};