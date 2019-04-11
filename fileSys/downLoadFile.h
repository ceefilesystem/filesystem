#pragma once
#include "mapFile.h"

class downLoadFile
{
private:
	readByMapFile* rMF;

	const char* fileName;
	size_t tolSize;

public:
	downLoadFile(const char* _filename);
	~downLoadFile();

	size_t getTolSize();
	int downLoadByRange(int pos, int count, char** out);
};
