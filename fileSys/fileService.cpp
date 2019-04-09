#include "upLoadFile.h"
#include "downLoadFile.h"
#include "fileService.h"
#include "readConfig.h"

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
}

FSByHttp::~FSByHttp()
{
}

int FSByHttp::serviceInit()
{
	return 0;
}

void FSByHttp::serviceDestroy()
{
}

void FSByHttp::startService()
{
}

void FSByHttp::stopService()
{
}

void FSByHttp::canceService()
{
}


//////////////////////////websocket //////////////////////////

FSByWebSocket::FSByWebSocket()
{

}

FSByWebSocket::~FSByWebSocket()
{
}

int FSByWebSocket::serviceInit()
{
	return 0;
}

void FSByWebSocket::serviceDestroy()
{
}

void FSByWebSocket::startService()
{
}

void FSByWebSocket::stopService()
{
}

void FSByWebSocket::canceService()
{
}
