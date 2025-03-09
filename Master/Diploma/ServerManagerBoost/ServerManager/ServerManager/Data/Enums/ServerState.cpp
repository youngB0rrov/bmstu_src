#include "ServerState.h"

std::ostream& operator<<(std::ostream& os, ServerState serverState)
{
	switch (serverState)
	{
	case ServerState::LOBBY:
		os << "LOBBY";
		break;
	case ServerState::MATCH_STARTING:
		os << "MATCH_STARTING";
		break;
	case ServerState::MATCH_IN_PROGRESS:
		os << "MATCH_IN_PROGRESS";
		break;
	case ServerState::RESTARTING:
		os << "RESTARTING";
		break;
	case ServerState::STUTTING_DOWN:
		os << "STUTTING_DOWN";
		break;
	default:
		os << "UNKNOWN";
		break;
	}
	return os;
}
