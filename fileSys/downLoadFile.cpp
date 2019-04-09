#include "downLoadFile.h"

downLoadFile::downLoadFile(const char * _filename)
{
	if (_filename == nullptr)
		throw "create writemapfile error";
	else
		if (!checkFileIsExist(_filename)) //不存在文件
			throw "create writemapfile error";

	this->fileName = _filename;
	this->rMF = new readByMapFile(fileName);
	if (!rMF)
		throw "create writemapfile error";
	else
		this->tolSize = rMF->getTolSize();
}

downLoadFile::~downLoadFile()
{
	if (rMF) {
		delete this->rMF;
		this->rMF = nullptr;
	}
}

int downLoadFile::downLoad(char ** out)
{
	int ret = 0;
	if (tolSize < FILESIZE)
		ret = downLoadAll(out);
	else
		ret = downLoadByRange(0, 1024, out);

	return ret;
}

int downLoadFile::downLoadAll(char** out)
{
	int ret = this->rMF->readDate(0, tolSize, out);
	tolSize = 0;
	
	return ret;
}

int downLoadFile::downLoadByRange(int pos, int count, char ** out)
{
	int ret = this->rMF->readDate(pos, count, out);
	tolSize -= count;
	return ret;
}
