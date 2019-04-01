#include "MappedFileWin.h"


TMappedFile::TMappedFile(const char * filePath)
{

	// 映射文件到内存
	_dumpFileDescriptor = CreateFileA(filePath,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (_dumpFileDescriptor == INVALID_HANDLE_VALUE)
		return;

	_fileMappingObject = CreateFileMapping(_dumpFileDescriptor,
								NULL,
								PAGE_READWRITE,
								0,
								0,
								NULL);
	if (_fileMappingObject == NULL)
		return;
	_mappedFileSize = GetFileSize(_fileMappingObject, NULL);
	_mappedFileAddress = MapViewOfFile(_fileMappingObject,
								FILE_MAP_ALL_ACCESS,
								0,
								0,
								_mappedFileSize);
	 
}


TMappedFile::~TMappedFile()
{
	UnmapViewOfFile(_mappedFileAddress);
	CloseHandle(_fileMappingObject);
	CloseHandle(_dumpFileDescriptor);

}

int TMappedFile::readRang(int pos, int count, void * out)
{
	out = NULL;
	if (_mappedFileAddress == NULL)
		return -1;
	if (pos < 0)
		return -1;
	if ((pos > _mappedFileSize - 1) || (pos + count > _mappedFileSize - 1))
		return -1;
	out = (void *)((size_t)_mappedFileAddress + pos);
	if (pos + count < _mappedFileSize - 1)
		return count;
	else
		return (_mappedFileSize - 1 - pos);
}

int TMappedFile::writeRang(int pos, int size, void * in)
{
	if (_mappedFileAddress == NULL)
		return -1;
	if (pos < 0)
		return -1;
	if ((pos > _mappedFileSize - 1) || (pos + size > _mappedFileSize - 1))
		return -1;
	memcpy((void *)((size_t)_mappedFileAddress + pos), in, size);
	return size;
}

int TMappedFile::mappedFileSize()
{
	return _mappedFileSize;
}
