#ifndef _MAPPED_FILE_WIN_H_
#define _MAPPED_FILE_WIN_H_
/*
	ӳ���ļ���  ӳ���ļ����ڴ� �ṩ��ȡ����
*/
#if _WIN32
#include <windows.h>
#endif

void* CreateMappedFile(const char * filePath);
int mappedFileReadRang(void * mappedFile, int pos, int count, void * out);
int mappedFileWriteRang(void * mappedFile, int pos, int size, void * in);
int CloseMappedFIle(void * mappedFile);

#endif

