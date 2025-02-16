#include <string>
#include "../../Data/Enums/IncomeServerManagerCommand.h"

class CommandsHelper
{
public:
	static IncomeServerManagerCommand GetIncomeServerManagerCommand(const std::string& command);
};