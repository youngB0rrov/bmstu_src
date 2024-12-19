#include <sstream>
#include <iostream>
#include "CommandsHelper.h"


std::string CommandsHelper::GetCommandTypeFromMessage(const std::string& message)
{
    std::istringstream stream(message);
    std::string command;
    std::getline(stream, command, ',');

    return command;
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
        else
        {
            std::cout << "Char \'=\' not found. Probably message command type: " << token << std::endl;
        }
    }

    return keyValuePairs;
}
