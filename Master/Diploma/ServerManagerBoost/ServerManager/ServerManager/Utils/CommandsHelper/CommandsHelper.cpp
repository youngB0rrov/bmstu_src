#include <sstream>
#include <iostream>
#include "CommandsHelper.h"
#include "../../Data/Enums/ServerCommandType.h"
#include "../../Data/Enums/ServerState.h"


std::string CommandsHelper::GetCommandTypeFromMessage(const std::string& message)
{
    std::istringstream stream(message);
    std::string command;
    std::getline(stream, command, ',');

    return command;
}

ServerCommandType CommandsHelper::GetServerCommandType(const std::string& message)
{
    std::string stringCommand = GetCommandTypeFromMessage(message);

    static const std::unordered_map<std::string, ServerCommandType> commandMap =
    {
        { "REGISTER_SERVER", ServerCommandType::REGISTER_SERVER },
        { "UPDATE_SERVER", ServerCommandType::UPDATE_SERVER }
    };

    auto iterator = commandMap.find(stringCommand);

    if (iterator != commandMap.end())
    {
        return iterator->second;
    }
    
    return ServerCommandType::UNKNOWN;
}

ServerState CommandsHelper::GetServerStateType(const std::string& message)
{
    static const std::unordered_map<std::string, ServerState> stateMap =
    {
        { "LOBBY", ServerState::LOBBY },
        { "MATCH_STARTING", ServerState::MATCH_STARTING },
        { "MATCH_IN_PROGRESS", ServerState::MATCH_IN_PROGRESS },
        { "STUTTING_DOWN", ServerState::STUTTING_DOWN },
        { "RESTARTING", ServerState::RESTARTING },
        { "UNKNOWN", ServerState::UNKNOWN },
    };

    auto iterator = stateMap.find(message);

    if (iterator != stateMap.end())
    {
        return iterator->second;
    }

    return ServerState::UNKNOWN;
}

std::unordered_map<std::string, std::string> CommandsHelper::GetKeyValuePairs(const std::string& message)
{
    std::unordered_map<std::string, std::string> keyValuePairs;
    std::istringstream stream(message);
    std::string token;

    while (std::getline(stream, token, ','))
    {
        size_t pos = token.find('=');
        if (pos != std::string::npos)
        {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            keyValuePairs[key] = value;
        }
    }

    return keyValuePairs;
}
