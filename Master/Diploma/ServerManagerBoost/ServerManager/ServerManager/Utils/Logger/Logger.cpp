#include "Logger.h"
#include <ctime>
#include <iomanip>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
}

void Logger::SetLogFile(const std::string& logDirectory)
{
	boost::gregorian::date todayDate = boost::gregorian::day_clock::local_day();
	std::string fileName = boost::gregorian::to_iso_extended_string(todayDate) + ".log";

	boost::filesystem::path dirPath = boost::filesystem::absolute(logDirectory);
	boost::filesystem::path filePath = boost::filesystem::path(logDirectory) / fileName;

	if (!boost::filesystem::exists(dirPath))
	{
		if (!boost::filesystem::create_directory(dirPath))
		{
			std::string errorMessage = "Failed to create log directory: " + dirPath.string();
			throw std::runtime_error(errorMessage);
		}
	}

	fileStream.open(filePath.string(), std::ios::app);
	if (!fileStream.is_open())
	{
		throw std::runtime_error("Failed to open log file");
	}
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

#ifdef _WIN32
	localtime_s(&timeInfo, &nowTime);
#else
	localtime_r(&nowTime, &timeInfo);
#endif

	std::ostringstream timeStampStream;

	timeStampStream << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S")
		<< "." << std::setfill('0') << std::setw(3) << millisecondsCount
		<< "] ";
	std::string timeStamp = timeStampStream.str();

	out << timeStamp << message;
	if (fileStream.is_open())
	{
		fileStream << timeStamp << message;
		fileStream.flush();
	}
}
