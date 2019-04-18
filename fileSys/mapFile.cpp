#include "mapFile.h"
#include <iostream>
#include <fstream>

uMapFile mapFile::refReadMap;
uMapFile mapFile::refWriteMap;
static std::mutex rLock;
static std::mutex wLock;

mapFile::mapFile()
{
	mapFileAddr = nullptr;
	mapFileSize = 0;
	mFile = nullptr;
	mReg = nullptr;

	rTolSize = 0;
	wTolSize = 0;
}

mapFile::~mapFile()
{
	mapFileAddr = nullptr;
	mapFileSize = 0;

	if (mFile) {
		delete mFile;
		mFile = nullptr;
	}

	if (mReg) {
		delete mReg;
		mReg = nullptr;
	}

	rTolSize = 0;
	wTolSize = 0;
}

void mapFile::createMapFile(const char * _filename, size_t _tolSize, size_t begpos, size_t offset)
{
	if (_filename == nullptr)
		return;

	try
	{
		//创建文件的内存映射
		this->mFile = new file_mapping(_filename, read_write);
		//在本进程中将全部的共享内存对象映射到映射域中
		this->mReg = new mapped_region(*this->mFile, read_write, begpos, offset);

		//通过生成的映射域对象，可以获取该共享内存的起始地址 和 该共享内存的大小(字节)
		this->mapFileAddr = (char*)mReg->get_address();
		this->mapFileSize = mReg->get_size();

		if (_tolSize == 0)
			rTolSize = this->mapFileSize;
		else
			wTolSize = _tolSize;

	}
	catch (const std::exception& e)
	{
		throw e;
	}

	return;
}

int mapFile::mapFileReadRang(int pos, int count, char ** out)
{
	if (this->mapFileAddr == NULL || pos < 0)
		return -1;
	if (pos > this->mapFileSize)
		return -1;

	*out = this->mapFileAddr + pos;

	if (pos + count < this->mapFileSize)
		return count;
	else
		return (this->mapFileSize - pos);
}

int mapFile::mapFileWriteRang(int pos, int count, const char * in)
{
	if (this->mapFileAddr == NULL || in == NULL || pos < 0)
		return -1;
	if ((pos > this->mapFileSize) || (pos + count > this->mapFileSize))
		return -1;

	memcpy(this->mapFileAddr + pos, in, count);

	return count;
}

readByMapFile::readByMapFile(const char * _fileName, size_t begpos, size_t offset)
{
	if (_fileName)
		fileName = _fileName;

	try
	{
		if (mapFile::refReadMap[_fileName] == nullptr) {
			mf = new mapFile();
			//查看是否创建映射
			mf->createMapFile(_fileName);
			rLock.lock();
			mf->refReadMap[_fileName] = mf;
			rLock.unlock();
			mf->refReadCount++;
		}
		else {
			rLock.lock();
			mf = mapFile::refReadMap[_fileName];
			rLock.unlock();
			mf->refReadCount++;
		}

		tolSize = mf->rTolSize;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

readByMapFile::~readByMapFile()
{
	if (--mf->refReadCount == 0) {
		rLock.lock();
		delete mf;
		mf = nullptr;
		mapFile::refReadMap.erase(fileName);
		rLock.unlock();
	}
}

int readByMapFile::readDate(int pos, int count, char ** out)
{
	return mf->mapFileReadRang(pos, count, out);
}

int readByMapFile::getUseCout()
{
	return mf->refReadCount;
}

size_t readByMapFile::getTolSize()
{
	return this->tolSize;
}

writeByMapFile::writeByMapFile(const char * _fileName, size_t _tolSize, size_t begpos, size_t offset)
{
	if (_fileName)
		fileName = _fileName;

	try
	{
		if (mapFile::refWriteMap[_fileName] == nullptr) {
			mf = new mapFile();
			//查看是否创建映射
			mf->createMapFile(_fileName, _tolSize);
			wLock.lock();
			mf->refWriteMap[_fileName] = mf;
			wLock.unlock();
			mf->refWriteCount++;
		}
		else {
			wLock.lock();
			mf = mapFile::refWriteMap[_fileName];
			wLock.unlock();
			mf->refWriteCount++;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

writeByMapFile::~writeByMapFile()
{
	if (--mf->refWriteCount == 0 && mf->wTolSize == 0) {
		wLock.lock();
		delete mf;
		mf = nullptr;
		mapFile::refWriteMap.erase(fileName);
		wLock.unlock();
	}
}

int writeByMapFile::writeDate(int pos, int count, const char * in)
{
	return mf->mapFileWriteRang(pos, count, in);
}

int writeByMapFile::getUseCout()
{
	return mf->refWriteCount;
}

bool checkFileIsExist(const char* filename)
{
	return boost::filesystem::exists(filename);
}

std::string getCurrentFilePath(const char * filename)
{
	auto path = boost::filesystem::current_path().append(filename).string();
	return path;
}