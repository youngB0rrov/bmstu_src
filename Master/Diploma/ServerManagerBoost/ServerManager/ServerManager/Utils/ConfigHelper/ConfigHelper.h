#pragma once
#include <string>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include "../../Utils/Logger/Logger.h"

class ConfigHelper
{
public:
	template<typename T>
	static void ReadVariableFromConfig(const std::string& fileName, const std::string& variableName, T& variable);
};

template<typename T>
void ConfigHelper::ReadVariableFromConfig(const std::string& fileName, const std::string& variableName, T& variable)
{
	boost::program_options::options_description description("Configuration");
	boost::program_options::variables_map variableMap;

	try
	{
		description.add_options()(variableName.c_str(), boost::program_options::value<T>(&variable)->required());

		std::ifstream configFile(fileName);
		if (!configFile)
		{
			throw std::runtime_error("Unable to open config file!");
		}
		boost::program_options::store(boost::program_options::parse_config_file(configFile, description, true), variableMap);
		boost::program_options::notify(variableMap);

		if (variableMap.count(variableName))
		{
			variable = variableMap[variableName].as<T>();
		}
	}
	catch (const std::exception& e)
	{
		Logger::GetInstance() << "An exception ocurred, while reading variable " << variableName << " from config: " << e.what() << std::endl;
	}
}