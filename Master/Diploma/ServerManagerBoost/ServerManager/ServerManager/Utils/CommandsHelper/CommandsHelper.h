#include <string>
#include <unordered_map>
#include "../../Data/Enums/ServerCommandType.h"

class CommandsHelper
{
public:
	static std::unordered_map<std::string, std::string> GetKeyValuePairs(const std::string& message);
	static ServerCommandType GetServerCommandType(const std::string& message);

private:
	static std::string GetCommandTypeFromMessage(const std::string& message);
};