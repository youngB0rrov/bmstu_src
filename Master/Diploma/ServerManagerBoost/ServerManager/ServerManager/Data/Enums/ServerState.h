#pragma once

#include <ostream>
#include <string>

enum class ServerState
{
	LOBBY,
	MATCH_STARTING,
	MATCH_IN_PROGRESS,
	STUTTING_DOWN,
	RESTARTING,
	UNKNOWN
};

std::ostream& operator<<(std::ostream& os, ServerState serverState);
const char* ToString(const ServerState& serverState);