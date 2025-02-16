#pragma once

#include <ostream>
#include <iostream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>

class Logger 
{
public:
	Logger(std::ostream& output = std::cout): out(output) {}
	
	static Logger& GetInstance();

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
	std::ostream& out;
	std::ostringstream buffer;
	std::mutex _mutex;

	void FlushBuffer();
	void Log(const std::string& message);
};