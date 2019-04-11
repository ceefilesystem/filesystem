#include "upLoadFile.h"
#include <fstream>

upLoadFile::upLoadFile(const char * _filename, size_t _tolsize)
{
	if(_filename == nullptr || _tolsize <= 0)
		throw "create readmapfile error";

	this->fileName = _filename;
	this->tolsize = _tolsize;

	//创建空白文件
	if (!checkFileIsExist(_filename))
		if(!ceateFullFile())
			throw "create readmapfile error";

	this->wMF = new writeByMapFile(_filename, tolsize);
	if (!wMF)
		throw "create readmapfile error";
}

upLoadFile::~upLoadFile()
{
	if (wMF) {
		delete this->wMF;
		this->wMF = nullptr;
	}
}

int upLoadFile::upLoadByRange(int pos, int count, const char* in)
{
	if (this->tolsize == 0)
		return 0;

	int ret = this->wMF->writeDate(pos, count, in);
	this->tolsize -= count;
	return ret;
}

bool upLoadFile::ceateFullFile(size_t offset)
{
	if (offset == 0)
		offset = tolsize;

	try
	{
		std::filebuf fbuf;
		fbuf.open(fileName, std::ios_base::in | std::ios_base::out
			| std::ios_base::trunc | std::ios_base::binary);

		fbuf.pubseekoff(offset - 1, std::ios_base::beg);
		fbuf.sputc(0);
		fbuf.close();
	}
	catch (const std::exception&)
	{
		return false;
	}
	
	return true;
}