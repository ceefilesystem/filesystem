#include "upLoadFile.h"
#include "downLoadFile.h"
#include "fileService.h"
#include "readConfig.h"
#include "uvServer.h"
#include "wsServer.h"
#include "httpServer.h"

//解析
//成功返回 >0
//失败返回 -1
static int downLoadFun(void* info, void** out)
{
	printf("%s\n", (char*)info);

	downLoadInof* dinfo = (downLoadInof*)info;
	downLoadFile* dfile = new downLoadFile(dinfo->filename);

	int ret = dfile->downLoadByRange(dinfo->pos, dinfo->count, (char**)out);
	if (*out != nullptr) {
		delete dfile;
		return ret;
	}
	else {
		delete dfile;
		return -1;
	}
}

//解析
//成功返回 >0
//失败返回 -1
static int upLoadFun(void* in)
{
	printf("%s\n", (char*)in);

	upLoadInof* uinfo = (upLoadInof*)in;
	upLoadFile* ufile = new upLoadFile(uinfo->filename, uinfo->totalSize);
	int ret = ufile->upLoadByRange(uinfo->pos, uinfo->count, (char*)in);
	if (ret != 0) {
		delete ufile;
		return ret;
	}
	else {
		delete ufile;
		return -1;
	}
}

/////////////////////////////////uv tcp////////////////

FSByTcp::FSByTcp()
{
	this->serverIp = "";
	this->serverPort = 0;
	this->us = nullptr;
}

FSByTcp::~FSByTcp()
{
	serviceDestroy();
}

int FSByTcp::serviceInit()
{
	try
	{
		readConfig rc(getCurrentFilePath("config.ini").c_str());

		this->serverIp = rc.getIP();
		this->serverPort = rc.getPort();

		this->us = new uvServer();
		return 0;
	}
	catch (...)
	{
		return 1;
	}
	
}

void FSByTcp::serviceDestroy()
{
	if (this->us)
		delete this->us;
}

int FSByTcp::startService()
{
	int r = serviceInit();
	if (r)
		return 1;

	this->us->setDownCallBack(downLoadFun);
	this->us->setUpCallBack(upLoadFun);

	r = this->us->start(this->serverIp.c_str(), this->serverPort);
	if (r)
		return 1;

	this->us->run();

	return 0;
}

int FSByTcp::stopService()
{
	return 0;
}

int FSByTcp::canceService()
{
	return 0;
}

//////////////////////////http //////////////////////////

FSByHttp::FSByHttp()
{
	this->serverIp = "";
	this->serverPort = 0;
	this->hs = nullptr;
}

FSByHttp::~FSByHttp()
{
	serviceDestroy();
}

int FSByHttp::serviceInit()
{
	try
	{
		readConfig rc(getCurrentFilePath("config.ini").c_str());

		this->serverIp = rc.getIP();
		this->serverPort = rc.getPort();

		this->hs = new httpServer();
		return 0;
	}
	catch (...)
	{
		return 1;
	}
	
}

void FSByHttp::serviceDestroy()
{
	if (this->hs)
		delete this->hs;
}

int FSByHttp::startService()
{
	int r = serviceInit();
	if (r)
		return 1;

	r = this->hs->start(this->serverIp.c_str(), this->serverPort);
	if (r)
		return 1;

	this->hs->run();
	return 0;
}

int FSByHttp::stopService()
{
	return 0;
}

int FSByHttp::canceService()
{
	return 0;
}


//////////////////////////websocket //////////////////////////

FSByWebSocket::FSByWebSocket()
{
	this->serverIp = "";
	this->serverPort = 0;
	this->ws = nullptr;
}

FSByWebSocket::~FSByWebSocket()
{
	serviceDestroy();
}

int FSByWebSocket::serviceInit()
{
	try
	{
		readConfig rc(getCurrentFilePath("config.ini").c_str());

		this->serverIp = rc.getIP();
		this->serverPort = rc.getPort();

		this->ws = new wsServer();
		return 0;
	}
	catch (...)
	{
		return 1;
	}
	
}

void FSByWebSocket::serviceDestroy()
{
	if (this->ws)
		delete this->ws;
}

int FSByWebSocket::startService()
{
	int r = serviceInit();
	if (r)
		return 1;

	r = this->ws->start(this->serverIp.c_str(), this->serverPort);
	if (r)
		return 1;

	this->ws->run();
	return 0;
}

int FSByWebSocket::stopService()
{
	return 0;
}

int FSByWebSocket::canceService()
{
	return 0;
}

void * initService(ProtocolType type)
{
	fileService* fs = nullptr;

	switch (type)
	{
	case 0:
	{
		fs = new FSByTcp();
		break;
	}
	case 1:
	{
		fs = new FSByTcp();
		break;
	}
	case 2:
	{
		fs = new FSByTcp();
		break;
	}
	default:
	{
		break;
	}
	}
	
	return fs;
}

void startService(void * handle)
{
	fileService* fs = (fileService*)handle;
	fs->startService();
}

void stopService(void * handle)
{
	fileService* fs = (fileService*)handle;
	fs->stopService();
}

void canceService(void * handle)
{
	fileService* fs = (fileService*)handle;
	fs->canceService();
}

void deleteService(void * handle)
{
	delete (fileService*)handle;
}
