#pragma once

#include <ostream>
#include <iostream>

class Logger {
public:
	Logger(std::ostream& output = std::cout): out(output) {}

	void Log(const std::string& message);

private:
	std::ostream& out;
};

extern Logger Log;