#include "CommandsHelper.h"
#include <unordered_map>
#include "../../Data/Enums/IncomeServerManagerCommand.h"

IncomeServerManagerCommand CommandsHelper::GetIncomeServerManagerCommand(const std::string& command)
{
    static const std::unordered_map<std::string, IncomeServerManagerCommand> commandMap =
    {
        { "START", IncomeServerManagerCommand::START },
        { "KILL", IncomeServerManagerCommand::KILL }
    };

    auto iterator = commandMap.find(command);
    if (iterator != commandMap.end())
    {
        return iterator->second;
    }

    return IncomeServerManagerCommand::UNKNOWN;
}
