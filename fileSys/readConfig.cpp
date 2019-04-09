#include "readConfig.h"

readConfig::readConfig(const char* configPath)
{
	read_ini(configPath, pt);
}

readConfig::~readConfig()
{
	
}

std::string readConfig::getIP()
{
	auto items = pt.get_child("net");
	return items.get<std::string>("ip");
}

int readConfig::getPort()
{
	auto items = pt.get_child("net");
	return items.get<int>("port");
}

std::string readConfig::getDefaultPath()
{
	auto items = pt.get_child("[defaultPath]");
	return items.get<std::string>("path");
}


