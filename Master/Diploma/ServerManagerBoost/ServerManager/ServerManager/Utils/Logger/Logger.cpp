#include "Logger.h"
#include <ctime>
#include <iomanip>

void Logger::Log(const std::string& message)
{
	std::time_t now = std::time(nullptr);
	std::tm timeInfo;
	localtime_s(&timeInfo, &now);
	out << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "]" << message << std::endl;
}

Logger Log;
