#pragma once
#include <ostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <string>

using namespace boost::property_tree;

class readConfig
{
private:
	ptree pt;

public:
	readConfig(const char* configPath);
	~readConfig();

	std::string getIP();
	int getPort();

	std::string getDefaultPath();
};
