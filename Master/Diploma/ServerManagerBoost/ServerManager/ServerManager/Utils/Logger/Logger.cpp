#include "Logger.h"
#include <ctime>
#include <iomanip>

Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
}

void Logger::FlushBuffer()
{
	Log(buffer.str());
	buffer.str("");
	buffer.clear();
}

void Logger::Log(const std::string& message)
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
	std::chrono::system_clock::duration milliseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
	std::chrono::system_clock::rep millisecondsCount = (milliseconds.count() / 1000) % 1000;

	std::tm timeInfo;
	localtime_s(&timeInfo, &nowTime);
	out << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S")
		<< "." << std::setfill('0') << std::setw(3) << millisecondsCount
		<< "] " << message;
}
