#pragma once
#include "mapFile.h"

class upLoadFile
{
private:
	writeByMapFile* wMF;

	const char* fileName;
	aSize_t tolsize;

	bool ceateFullFile(size_t offset = 0);

public:
	upLoadFile(const char* _filename, size_t _tolsize);
	~upLoadFile();

	//int upLoad(const char* in);

	////只允许被调用一次
	//int upLoadAll(const char* in);
	////可多次调用
	int upLoadByRange(int pos, int count, const char* in);
};

