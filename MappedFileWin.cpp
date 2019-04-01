#include "stdafx.h"
#include "MappedFileWin.h"

#if _WIN32
#include <windows.h>
#endif

typedef struct mappedFileWin
{
	const char* filePath;
	void * mappedFileAddress;
	size_t mappedFileSize;
	HANDLE dumpFileDescriptor;
	HANDLE fileMappingObject;
}mappedFileWin;

void* createMappedFile(const char * filePath)
{
	mappedFileWin* mappedFile;

	if (filePath)
		mappedFile->filePath = filePath;

	mappedFile->dumpFileDescriptor = CreateFileA(filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (mappedFile->dumpFileDescriptor == INVALID_HANDLE_VALUE)
		return NULL;

	mappedFile->fileMappingObject = CreateFileMapping(mappedFile->dumpFileDescriptor,
		NULL, PAGE_READWRITE, 0, 0, NULL);
	if (mappedFile->fileMappingObject == NULL) {
		CloseHandle(mappedFile->dumpFileDescriptor);
		return NULL;
	}

	mappedFile->mappedFileSize = GetFileSize(mappedFile->fileMappingObject, NULL);

	mappedFile->mappedFileAddress = MapViewOfFile(mappedFile->fileMappingObject,
		FILE_MAP_ALL_ACCESS, 0, 0, mappedFile->mappedFileSize);
	if (mappedFile->mappedFileAddress == NULL) {
		CloseHandle(mappedFile->fileMappingObject);
		CloseHandle(mappedFile->dumpFileDescriptor);
		return NULL;
	}

	return mappedFile;
}

int mappedFileReadRang(void * _mappedFile, int pos, int count, void * out)
{
	mappedFileWin* mappedFile = (mappedFileWin*)_mappedFile;
	if (mappedFile->mappedFileAddress == NULL)
		return -1;
	if (pos < 0)
		return -1;
	if ((pos > mappedFile->mappedFileSize - 1) || (pos + count > mappedFile->mappedFileSize - 1))
		return -1;
	out = (void *)((size_t)mappedFile->mappedFileAddress + pos);
	if (pos + count < mappedFile->mappedFileSize - 1)
		return count;
	else
		return (mappedFile->mappedFileSize - 1 - pos);
}

int mappedFileWriteRang(void * _mappedFile, int pos, int size, void * in)
{
	mappedFileWin* mappedFile = (mappedFileWin*)_mappedFile;
	if (mappedFile->mappedFileAddress == NULL || in == NULL)
		return -1;
	if (pos < 0)
		return -1;
	if ((pos > mappedFile->mappedFileSize - 1) || (pos + size > mappedFile->mappedFileSize - 1))
		return -1;
	memcpy((void *)((size_t)mappedFile->mappedFileAddress + pos), in, size);
	return size;
}

void closeMappedFile(void * _mappedFile)
{
	mappedFileWin* mappedFile = (mappedFileWin*)_mappedFile;

	UnmapViewOfFile(mappedFile->mappedFileAddress);
	CloseHandle(mappedFile->fileMappingObject);
	CloseHandle(mappedFile->dumpFileDescriptor);
	mappedFile = NULL;
}

size_t getFileSize(void * mappedFile)
{
	return ((mappedFileWin*)mappedFile)->mappedFileSize;
}

const char* getFilePath(void * mappedFile)
{
	((mappedFileWin*)mappedFile)->filePath;
}