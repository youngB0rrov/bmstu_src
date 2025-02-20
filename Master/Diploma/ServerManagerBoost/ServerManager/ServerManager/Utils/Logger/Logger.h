#pragma once

#include <ostream>
#include <iostream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>

class Logger 
{
public:
	Logger(std::ostream& output = std::cout): out(output) {}
	
	static Logger& GetInstance();

	void SetLogFile(const std::string& logDirectory);

	template<typename T>
	Logger& operator<<(const T& message)
	{
		buffer << message;
		return *this;
	}

	Logger& operator<<(std::ostream& (*manipulator)(std::ostream&))
	{
		std::lock_guard<std::mutex> lock(_mutex);
		buffer << manipulator;
		if (manipulator == static_cast<std::ostream & (*)(std::ostream&)>(std::endl)) {
			FlushBuffer();
		}
		return *this;
	}

private:
	std::ostream& out; // Standard ouput stream
	std::ostringstream buffer; // Buffer for all log message
	std::ofstream fileStream; // Output file stream
	std::mutex _mutex;

	void FlushBuffer();
	void Log(const std::string& message);
};