#include <string>
#include <unordered_map>

class CommandsHelper
{
public:
	static std::string GetCommandTypeFromMessage(const std::string& message);
	static std::unordered_map<std::string, std::string> GetKeyValuePairs(const std::string& message);
};