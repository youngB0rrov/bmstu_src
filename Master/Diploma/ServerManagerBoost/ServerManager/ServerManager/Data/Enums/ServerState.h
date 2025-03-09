#pragma once

#include <ostream>

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