#pragma once
#include <ostream>

enum class ClientType
{
	INITIATOR,
	PLAYER
};

std::ostream& operator<<(std::ostream& os, ClientType clientType);