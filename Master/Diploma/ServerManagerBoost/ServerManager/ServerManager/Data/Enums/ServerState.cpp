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

const char* ToString(const ServerState& serverState)
{
	switch (serverState)
	{
		case ServerState::LOBBY:
			return "LOBBY";
		case ServerState::MATCH_STARTING:
			return "MATCH_STARTING";
		case ServerState::MATCH_IN_PROGRESS:
			return "MATCH_IN_PROGRESS";
		case ServerState::STUTTING_DOWN:
			return "STUTTING_DOWN";
		case ServerState::RESTARTING:
			return "RESTARTING";
		default:
			return "UNKNOWN";
	}
}
