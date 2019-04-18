#define _CRT_SECURE_NO_WARNINGS
#include "upLoadFile.h"
#include "downLoadFile.h"
#include "fileService.h"
#include "readConfig.h"
#include "uvServer.h"
#include "wsServer.h"
#include "httpImpl.h"
#include "httpServer.h"
#include "protocol.h"
#include "timeWheel.h"

//#define TIMERTIME 1000 * 60 * 5
#define TIMERTIME 1000

static TimeWheel* tw;
static timerID timer_id;

//回调
static void timerfun();

//tcp协议回调
static int tcpDownLoadFun(void* in, void** out);
static int tcpUpLoadFun(void* in);
//http协议回调
static int httpDownLoadFun(void* in, void** out);
static int httpUpLoadFun(void* in);
//websocket协议回调
static int websocketDownLoadFun(void* in, void** out);
static int websocketUpLoadFun(void* in);

//定时器回调函数
static void timerfun()
{
	//检测上传文件是否中断
	//printf("timerfun\n");
	return;
}

//解析回调
//成功返回 >0
//失败返回 -1
static int tcpDownLoadFun(void* info, void** out)
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

//解析回调
//成功返回 >0
//失败返回 -1
static int tcpUpLoadFun(void* in)
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

	this->us->setDownCallBack(tcpDownLoadFun);
	this->us->setUpCallBack(tcpUpLoadFun);

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

static int httpDownLoadFun(void* in, void** out)
{
	httpRequest* request = (httpRequest*)in;

	std::string fileName = "";
	size_t pos = 0;
	size_t end = 0;
	size_t count = 0;

	//解析
	fileName = request->httpUrl.substr(1, request->httpUrl.find('?', 1) - 1);
	sscanf((char*)(request->httpHeaders["Range"].c_str()), "bytes=%lld-%lld", &pos, &end);

	count = end - pos;
	downLoadFile* dfile = new downLoadFile(fileName.c_str());

	char* outbuf = nullptr;
	int ret = dfile->downLoadByRange(pos, count, (char**)&outbuf);
	if (outbuf != nullptr) {
		*out = calloc(1, count);
		memcpy(*out, outbuf, count);
		delete dfile;
		return  ret;
	}
	else {
		delete dfile;
		return -1;
	}
	return 0;
}

static int httpUpLoadFun(void* in)
{
	httpRequest* request = (httpRequest*)in;

	std::string fileName = "";
	size_t totalSize = 0;
	size_t pos = 0;
	size_t count = 0;

	//解析
	fileName = request->httpUrl.substr(1, request->httpUrl.find('?', 1));
	sprintf((char*)request->httpHeaders["Range"].c_str(), "bytes=%lld-%lld", &pos, &count);
	sprintf((char*)request->httpHeaders["TotalSize"].c_str(), "%lld", &totalSize);

	upLoadFile* ufile = new upLoadFile(fileName.c_str(), totalSize);
	int ret = ufile->upLoadByRange(pos, count, (char*)in);
	if (ret != 0) {
		delete ufile;
		return ret;
	}
	else {
		delete ufile;
		return -1;
	}

	return 0;
}

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

	this->hs->setDownCallBack(httpDownLoadFun);
	this->hs->setUpCallBack(httpUpLoadFun);
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

static int websocketDownLoadFun(void* info, void** out)
{

	return 0;
}

static int websocketUpLoadFun(void* in)
{

	return 0;
}

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

	this->ws->setDownCallBack(websocketDownLoadFun);
	this->ws->setUpCallBack(websocketUpLoadFun);
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

///////////////////调用接口函数//////////////////

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
			fs = new FSByHttp();
			break;
		}
		case 2:
		{
			fs = new FSByWebSocket();
			break;
		}
		default:
		{
			break;
		}
	}

	tw = createTimerWheel(1000, 10);
	timer_id = addTimerWheel(tw, TIMERTIME, timerfun);

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
	deleteTimer(tw, timer_id);
	delete (fileService*)handle;
}
