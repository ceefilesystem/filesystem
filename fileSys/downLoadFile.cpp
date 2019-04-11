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

size_t downLoadFile::getTolSize()
{
	return tolSize;
}

int downLoadFile::downLoadByRange(int pos, int count, char ** out)
{
	return this->rMF->readDate(pos, count, out);
}
