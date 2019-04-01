#ifndef _MAPPED_FILE_WIN_H_
#define _MAPPED_FILE_WIN_H_
/*
	映射文件类  映射文件到内存 提供读取操作
*/
#if _WIN32
#include <windows.h>
#endif

void* CreateMappedFile(const char * filePath);
int mappedFileReadRang(void * mappedFile, int pos, int count, void * out);
int mappedFileWriteRang(void * mappedFile, int pos, int size, void * in);
int CloseMappedFIle(void * mappedFile);

#endif

