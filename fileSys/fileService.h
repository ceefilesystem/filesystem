#pragma once
#include <string>
#include "uvServer.h"
#include "wsServer.h"

class fileService
{
public:
	virtual int serviceInit() = 0;
	virtual void serviceDestroy() = 0;

	virtual void startService() = 0;
	virtual void stopService() = 0;
	virtual void canceService() = 0;
};

//////////////////////////tcp //////////////////////////
class FSByTcp : public fileService
{
private:
	std::string serverIp;
	int serverPort;
	uvServer* us;

public:
	FSByTcp();
	virtual ~FSByTcp();

	virtual int serviceInit();
	virtual void serviceDestroy();

	virtual void startService();
	virtual void stopService();
	virtual void canceService();
};

//////////////////////////http //////////////////////////
class FSByHttp : public fileService
{
private:
	std::string serverIp;
	int serverPort;

	wsServer* ws;

public:
	FSByHttp();
	virtual ~FSByHttp();

	virtual int serviceInit();
	virtual void serviceDestroy();

	virtual void startService();
	virtual void stopService();
	virtual void canceService();
};

//////////////////////////websocket //////////////////////////
class FSByWebSocket : public fileService
{
private:
	std::string serverIp;
	int serverPort;

	wsServer* ws;

public:
	FSByWebSocket();
	virtual ~FSByWebSocket();

	virtual int serviceInit();
	virtual void serviceDestroy();

	virtual void startService();
	virtual void stopService();
	virtual void canceService();
};
