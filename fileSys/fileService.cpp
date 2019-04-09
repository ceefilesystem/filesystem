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
static int downLoadFun(void* in, void** out)
{
	printf("%s\n", (char*)in);

	downLoadInof* dinfo = (downLoadInof*)in;
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
	readConfig rc(getCurrentFilePath("config.ini").c_str());

	this->serverIp = rc.getIP();
	this->serverPort = rc.getPort();

	this->us = new uvServer();
	return 0;
}

void FSByTcp::serviceDestroy()
{
	if (this->us)
		delete this->us;
}

void FSByTcp::startService()
{
	serviceInit();

	this->us->setDownCallBack(downLoadFun);
	this->us->setDownCallBack(downLoadFun);
	this->us->start(this->serverIp.c_str(), this->serverPort);
	this->us->run();
}

void FSByTcp::stopService()
{
	return;
}

void FSByTcp::canceService()
{
	return;
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
	readConfig rc(getCurrentFilePath("config.ini").c_str());

	this->serverIp = rc.getIP();
	this->serverPort = rc.getPort();

	this->hs = new httpServer();
	return 0;
}

void FSByHttp::serviceDestroy()
{
	if (this->hs)
		delete this->hs;
}

void FSByHttp::startService()
{
	serviceInit();

	this->hs->start(this->serverIp.c_str(), this->serverPort);
	this->hs->run();
}

void FSByHttp::stopService()
{
	return;
}

void FSByHttp::canceService()
{
	return;
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
	readConfig rc(getCurrentFilePath("config.ini").c_str());

	this->serverIp = rc.getIP();
	this->serverPort = rc.getPort();

	this->ws = new wsServer();
	return 0;
}

void FSByWebSocket::serviceDestroy()
{
}

void FSByWebSocket::startService()
{
	serviceInit();

	this->ws->start(this->serverIp.c_str(), this->serverPort);
	this->ws->run();
}

void FSByWebSocket::stopService()
{
	return;
}

void FSByWebSocket::canceService()
{
	return;
}
