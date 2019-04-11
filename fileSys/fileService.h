#pragma once
#include <string>

class fileService
{
	virtual int serviceInit() = 0;
	virtual void serviceDestroy() = 0;

public:
	fileService() {};
	virtual ~fileService() {};
	virtual int startService() = 0;
	virtual int stopService() = 0;
	virtual int canceService() = 0;
};

class uvServer;
class wsServer;
class httpServer;

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

	virtual int startService();
	virtual int stopService();
	virtual int canceService();

protected:
	virtual int serviceInit();
	virtual void serviceDestroy();
};

//////////////////////////http //////////////////////////
class FSByHttp : public fileService
{
private:
	std::string serverIp;
	int serverPort;

	httpServer* hs;

public:
	FSByHttp();
	virtual ~FSByHttp();

	virtual int startService();
	virtual int stopService();
	virtual int canceService();

protected:
	virtual int serviceInit();
	virtual void serviceDestroy();
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

	virtual int startService();
	virtual int stopService();
	virtual int canceService();

protected:
	virtual int serviceInit();
	virtual void serviceDestroy();
};

enum ProtocolType;
extern void* initService(ProtocolType type);
extern void startService(void* handle);
extern void stopService(void* handle);
extern void canceService(void* handle);
extern void deleteService(void* handle);