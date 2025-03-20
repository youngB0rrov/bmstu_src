#pragma once

#include <ostream>
#include <string>

enum class ServerState
{
	LOBBY,
	MATCH_STARTING,
	MATCH_IN_PROGRESS,
	SHTUTTING_DOWN,
	RESTARTING,
	UNKNOWN
};

std::ostream& operator<<(std::ostream& os, ServerState serverState);
std::string ToString(const ServerState& serverState);