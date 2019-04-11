#pragma once
#include <string>

struct file_update_info
{
	char md5[32];
	int totalSize;
	int fileNameSize;
	char* fileName;
};

struct file_download_info
{
	int fileNameSize;
	char* fileName;
};

// md5   ---  filePath
struct file_data
{
	char  md5[32];
	int posSize;
	int rangSize;
	void * data;
};

typedef struct downLoadInof {
	const char* filename;
	int pos;
	int count;
}downLoadInof;

typedef struct upLoadInof {
	const char* filename;
	int pos;
	int count;
	int totalSize;
}upLoadInof;

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
//extern void setReadCallBack(void* handle, int(*readCallBack) (char*, int));
//extern int sendData(void* handle, const char* buf, int len);
extern void deleteService(void* handle);