#pragma once
#include "mapFile.h"

class downLoadFile
{
private:
	readByMapFile* rMF;

	const char* fileName;
	aSize_t tolSize;

public:
	downLoadFile(const char* _filename);
	~downLoadFile();

	int downLoadByRange(int pos, int count, char** out);
};
