#pragma once
#include <mutex>
#include <string>
#include <stdint.h>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

using namespace boost::interprocess;

class mapFile;
typedef boost::atomic_int aInt;
typedef boost::atomic_size_t aSize_t;
typedef boost::unordered_map<const char*, mapFile*> uMapFile;

//创建文件映射
class mapFile
{
private:
	char * mapFileAddr;
	size_t mapFileSize;

	file_mapping* mFile;
	mapped_region* mReg;

public:
	size_t rTolSize;
	aInt refReadCount;
	static uMapFile refReadMap;

	size_t wTolSize;
	aInt refWriteCount;
	static uMapFile refWriteMap;

	mapFile();
	virtual ~mapFile();

	void createMapFile(const char * _filename, size_t tolSize = 0, size_t begpos = 0, size_t offset = 0);
	int mapFileReadRang(int pos, int count, char ** out);
	int mapFileWriteRang(int pos, int count, const char * in);
};

//读操作
class readByMapFile
{
private:
	const char* fileName;
	mapFile* mf;

	size_t tolSize;

public:
	readByMapFile(const char* _fileName, size_t begpos = 0, size_t offset = 0);
	~readByMapFile();
	
	int readDate(int pos, int count, char ** out);
	int getUseCout();
	size_t getTolSize();
};

//写操作
class writeByMapFile
{
private:
	const char* fileName;
	mapFile* mf;

public:
	writeByMapFile(const char* _fileName, size_t tolSize, size_t begpos = 0, size_t offset = 0);
	~writeByMapFile();

	int writeDate(int pos, int count, const char * in);
	int getUseCout();
};

extern bool checkFileIsExist(const char* filename);
extern std::string getCurrentFilePath(const char* filename);