#include "ClientType.h"

std::ostream& operator<<(std::ostream& os, ClientType clientType) {
	switch (clientType) 
	{
		case ClientType::INITIATOR:
			os << "INITIATOR";
			break;
		case ClientType::PLAYER:
			os << "PLAYER";
			break;
		default:
			os << "UNKNOWN";
			break;
	}
	return os;
}